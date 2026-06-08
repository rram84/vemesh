function mesh = read_vemesh_vtk(filepath)
% READ_VEMESH_VTK  Read a legacy-ASCII VTK POLYDATA mesh written by vemesh.
%
%   mesh = read_vemesh_vtk(filepath) returns a struct with fields
%       vertices : (n_vertices x 2) coordinates (x, y); z is dropped
%       elements : (1 x n_elements) cell array; each cell a row vector of
%                  1-based vertex indices, ordered counter-clockwise
%       boundary : column vector of 1-based indices of vertices on the outer
%                  (domain) boundary, found as endpoints of edges used by a
%                  single element
%
%   vemesh writes 0-based connectivity; it is shifted to 1-based here.

  fid = fopen(filepath, 'r');
  if fid < 0
    error('read_vemesh_vtk:open', 'Could not open file: %s', filepath);
  end
  cleaner = onCleanup(@() fclose(fid)); %#ok<NASGU>

  % ---- POINTS ----
  pts_line = find_keyword(fid, 'POINTS');
  tok = sscanf(pts_line, '%*[^0-9]%d');     % first integer on the line
  n_vertices = tok(1);
  raw = fscanf(fid, '%f', 3 * n_vertices);  % x y z per vertex
  raw = reshape(raw, 3, n_vertices).';
  mesh.vertices = raw(:, 1:2);              % 2D: drop z

  % ---- POLYGONS ----
  poly_line = find_keyword(fid, 'POLYGONS');
  tok = sscanf(poly_line, '%*[^0-9]%d');
  n_elements = tok(1);
  elements = cell(1, n_elements);
  for e = 1:n_elements
    valence = fscanf(fid, '%d', 1);
    conn    = fscanf(fid, '%d', valence);   % 0-based vertex indices
    elements{e} = conn(:).' + 1;            % 1-based row vector
  end
  mesh.elements = elements;

  % ---- boundary vertices: endpoints of edges used by exactly one element ----
  total = sum(cellfun(@numel, elements));
  edges = zeros(total, 2);
  pos = 0;
  for e = 1:n_elements
    v  = elements{e}(:);
    ne = numel(v);
    edges(pos+1:pos+ne, :) = [v, v([2:end, 1])];
    pos = pos + ne;
  end
  edges = sort(edges, 2);                    % undirected
  [ue, ~, ic] = unique(edges, 'rows');
  counts = accumarray(ic, 1);
  boundary_edges = ue(counts == 1, :);
  mesh.boundary = unique(boundary_edges(:));
end


function line = find_keyword(fid, keyword)
  while ~feof(fid)
    line = fgetl(fid);
    if ischar(line) && contains(line, keyword)
      return;
    end
  end
  error('read_vemesh_vtk:keyword', 'Keyword "%s" not found', keyword);
end
