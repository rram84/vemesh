function [ratio, lambda_max, lambda_min] = vem_quality(filepath)
% VEM_QUALITY  Conditioning of the pure-Neumann VEM stiffness matrix.
%
%   [ratio, lambda_max, lambda_min] = vem_quality(filepath) reads the vemesh
%   VTK mesh, assembles the k=1 VEM Poisson stiffness with unit stabilization,
%   and returns its conditioning under pure Neumann ("do nothing") boundary
%   conditions -- i.e. no DOFs are removed, the full assembled matrix is used.
%
%   With pure Neumann BCs the stiffness is only positive *semi*-definite: the
%   constant mode is in its null space, so the smallest eigenvalue is (up to
%   round-off) zero. The meaningful conditioning measure is therefore
%       ratio = lambda_max / lambda_2
%   where lambda_2 is the second-smallest (smallest nonzero) eigenvalue. The
%   returned lambda_min IS this second-smallest eigenvalue, not the ~0 one.

  mesh = read_vemesh_vtk(filepath);
  K = vem_stiffness(mesh, 1);               % stabilization factor = 1
  K = (K + K.') / 2;                        % kill round-off asymmetry

  % K is sparse and symmetric, so eigs uses the symmetric Lanczos path.
  % We need only two eigenvalues, so a full dense decomposition is avoided.
  lambda_max = eigs(K, 1, 'largestabs');

  % The constant mode makes the smallest eigenvalue ~ 0, so we ask for the two
  % eigenvalues nearest a tiny shift and keep the second: lambda_2, the smallest
  % nonzero eigenvalue. A plain 'smallestabs' would shift-invert at sigma = 0,
  % i.e. factor the EXACTLY singular K (RCOND ~ eps): MATLAB then warns and the
  % result is unreliable. A small NEGATIVE sigma makes K - sigma*I SPD and well
  % conditioned (RCOND ~ |sigma|/lambda_max ~ 1e-12, no warning), and the two
  % eigenvalues nearest ~0 are still the constant mode and lambda_2.
  sigma  = -1e-12 * lambda_max;
  small  = sort(eigs(K, 2, sigma));          % [~0 ; lambda_2]
  lambda_min = small(2);                      % second-smallest = smallest nonzero
  ratio  = lambda_max / lambda_min;
end


% ------------------------------------------------------------------------- %
function K = vem_stiffness(mesh, stab_factor)
% Global lowest-order (k=1) VEM stiffness matrix for -Laplacian.
% Local element stiffness follows O. Sutton, "The virtual element method in
% 50 lines of MATLAB", Numer. Algorithms 75 (2017). stab_factor scales the
% (I-Pi)'(I-Pi) stabilization term.

  if nargin < 2, stab_factor = 1; end

  n_dofs  = size(mesh.vertices, 1);
  n_polys = 3;                              % {1, x, y}
  K = sparse(n_dofs, n_dofs);
  linear_polynomials = {[0,0], [1,0], [0,1]};
  mod_wrap = @(x, a) mod(x-1, a) + 1;

  for el_id = 1:numel(mesh.elements)
    vert_ids = mesh.elements{el_id};
    verts    = mesh.vertices(vert_ids, :);
    n_sides  = numel(vert_ids);

    area_components = verts(:,1) .* verts([2:end,1],2) - verts([2:end,1],1) .* verts(:,2);
    area     = 0.5 * abs(sum(area_components));
    centroid = sum((verts + verts([2:end,1],:)) .* repmat(area_components,1,2)) / (6*area);

    diameter = 0;
    for i = 1:(n_sides-1)
      for j = (i+1):n_sides
        diameter = max(diameter, norm(verts(i,:) - verts(j,:)));
      end
    end

    D = zeros(n_sides, n_polys); D(:,1) = 1;
    B = zeros(n_polys, n_sides); B(1,:) = 1/n_sides;
    for vertex_id = 1:n_sides
      vert = verts(vertex_id, :);
      prev = verts(mod_wrap(vertex_id-1, n_sides), :);
      next = verts(mod_wrap(vertex_id+1, n_sides), :);
      vertex_normal = [next(2)-prev(2), prev(1)-next(1)];
      for poly_id = 2:n_polys
        poly_degree   = linear_polynomials{poly_id};
        monomial_grad = poly_degree / diameter;
        D(vertex_id, poly_id) = dot(vert - centroid, poly_degree) / diameter;
        B(poly_id, vertex_id) = 0.5 * dot(monomial_grad, vertex_normal);
      end
    end

    projector = (B*D) \ B;
    proj_err  = eye(n_sides) - D * projector;
    stabilising_term = stab_factor * (proj_err.' * proj_err);
    G = B*D; G(1,:) = 0;
    local_stiffness = projector.' * G * projector + stabilising_term;

    K(vert_ids, vert_ids) = K(vert_ids, vert_ids) + local_stiffness;
  end
end
