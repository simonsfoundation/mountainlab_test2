% get the path of this m-file
mfile_path=fileparts(mfilename('fullpath'))

% load the m-file and display its contents
mv_file=loadjson([mfile_path,'/example.mv']);
disp('The mv file as a matlab object:');
disp(mv_file);

% get all clusters that are accepted
accepted_clusters=get_mv_clusters_with_tag(mv_file,'accepted');
disp('accepted cluster numbers:')
disp(accepted_clusters);

% load the firings and then make a subset of firings that have accepted
% labels
firings=readmda('firings.mda');
firings2=ms_subfirings(firings,struct('clusters',accepted_clusters));
% here are the corresponding firing times:
times2=firings2(2,:);
disp('Number of events corresponding to accepted clusters:');
disp(length(times2));

% or if I want to get a single cluster:
firings3=ms_subfirings(firings,struct('clusters',13));
times3=firings3(2,:);
disp('Number of events corresponding to cluster 13:');
disp(length(times3));

% get the list of merged pairs (thx Loren)
pairs=get_mv_merged_pairs(mv_file);
disp('Merge pairs:');
for j=1:length(pairs)
    disp(pairs{j});
end;
