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

% get the list of merged pairs (thx Loren) -- not transitive
pairs=get_mv_merged_pairs(mv_file);
disp('Merge pairs:');
for j=1:length(pairs)
    disp(pairs{j});
end;

% get the list of merged groups -- transitive
groups=get_mv_merged_groups(mv_file);
disp('Merge groups:');
for j=1:length(groups)
    disp(groups{j});
end;Conversations


wehrli@mail.med.upenn.edu
Add to circles
wehrli@mail.med.upenn.edu


% get the merge matrix -- non-transitive
K=max(firings(3,:));
merge_matrix=get_mv_merge_matrix(mv_file,1:K,0);
figure; imagesc(merge_matrix); colormap('gray');
title('non-transitive');

% get the merge matrix -- transitive
K=max(firings(3,:));
merge_matrix=get_mv_merge_matrix(mv_file,1:K,1);
figure; imagesc(merge_matrix); colormap('gray');
title('transitive');


