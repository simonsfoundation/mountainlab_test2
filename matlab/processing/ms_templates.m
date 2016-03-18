function templates=ms_templates(clips,labels)
%MS_TEMPLATES - Compute templates (mean waveforms) corresponding to each
%spike type.
%
%Consider using mscmd_templates
%
% Syntax:  templates = ms_templates(clips,labels)
%
% Inputs:
%    clips - MxTxNC array of clips
%    labels - 1xNC array of integer labels
%
% Outputs:
%    templates - MxTxK array of computed templates, K=max(labels)
%
% Other m-files required: none
%
% See also: mscmd_templates

% Author: Jeremy Magland
% Jan 2015; Last revision: 13-Feb-2106

if (nargin<3) opts=struct; end;

[M,T,NC]=size(clips);

K=max(labels);

templates=zeros(M,T,K);

for k=1:K
    ii0=find(labels==k);
    clips0=clips(:,:,ii0);
    templates(:,:,k)=mean(clips0,3);
end;

end