function merged_pair_list=get_mv_merged_pairs(mv_file)

merged_pair_list={};
% set the delimiter for the pairs, which are currently stored as pair_n_m
delimiter = '_';

if (ischar(mv_file))
    mv_file=loadjson(mv_file);
end;

CPA=mv_file.cluster_pair_attributes;
names=fieldnames(CPA);
for j=1:length(names)
    if (~isempty(CPA.(names{j})) && contains_tag(CPA.(names{j}).tags,'merged'))
    	% parse the numbers of the merged cluster
	pair_num_strs = strsplit(names{j}, delimiter);
	% the first element will be 'pair' and the second and third elements will be the numbers
	pair_nums = [str2num(pair_num_strs{2}) str2num(pair_num_strs{3})];
        % go through the merge list and see if either of these clusters is in it already
    merged_pair_list{end+1}=pair_nums;
    
% 	if (length(merged_pair_list) == 0) 
% 	    % add the pair to the list
% 	    merged_pair_list{1} = pair_nums;
% 	else
% 	    found = 0;
% 	    for m_ind = 1:length(merged_pair_list)
% 	    	if ~isempty(intersect(merged_pair_list{m_ind}, pair_nums))
% 		    merged_pair_list{m_ind} = unique([merged_pair_list{m_ind} pair_nums]);
% 		    found = 1;
% 		    break;
% 		end
% 	    end
% 	    if ~found 
% 	       merged_pair_list{end+1} = pair_nums;
% 	   end
%     end
    end;
end;


function ret=contains_tag(list,tag)
for j=1:length(list)
    if (strcmp(list{j},tag))
        ret=1;
        return;
    end;
end;
ret=0;
