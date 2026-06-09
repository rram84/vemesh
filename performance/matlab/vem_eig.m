function vem_eig(varargin)
% VEM_EIG  Print VEM stiffness conditioning for one or more mesh files as CSV.
%
%   vem_eig(file1, file2, ...) writes one CSV line per file to stdout:
%       <name>,<lambda_min>,<lambda_max>,<ratio>
%
%   This is a thin, shell-friendly wrapper around the per-file kernel
%   vem_quality (which assembles the pure-Neumann, unit-stabilized k=1 VEM
%   stiffness and returns lambda_max, the smallest nonzero eigenvalue, and
%   their ratio -- the smallest eigenvalue itself is ~0, the constant mode). The
%   wrapper does no coordination: a shell driver decides which files belong
%   together and calls this once per group, e.g.
%
%       matlab -batch "addpath('.../matlab'); vem_eig('a.vtk','b.vtk')"

  for k = 1:nargin
    fp = varargin{k};
    [ratio, lambda_max, lambda_min] = vem_quality(fp);
    [~, base, ext] = fileparts(fp);
    fprintf('%s%s,%.17g,%.17g,%.17g\n', base, ext, lambda_min, lambda_max, ratio);
  end
end
