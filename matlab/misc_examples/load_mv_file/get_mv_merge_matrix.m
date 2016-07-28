function merge_matrix=get_mv_merge_matrix(mv_file,clusters,make_transitive)
% Returns a binary matrix reflecting the merges
% Clusters is a vector containing the numbers of the clusters you are
% interested in. For example clusters=1:50, or clusters=[5,6,7,8,12,15]
% If make_transitive=1, it will transitive-ify the matrix so that if a
% is merged with b and b with c, then a will be merged with c.

if (nargin<1) test_get_mv_merge_matrix; return; end; %self test
if (nargin<3) make_transitive=1; end;

K=length(clusters);
merge_matrix=eye(K,K);
pairs=get_mv_merged_pairs(mv_file);
for j=1:length(pairs)
    pair=pairs{j};
    i1=find(clusters==pair(1)); % get the index corresponding to clusters input vector
    i2=find(clusters==pair(2));
    if ((length(i1)==1)&&(length(i2)==1))
        merge_matrix(i1,i2)=1;
        merge_matrix(i2,i1)=1;
    end;
end;

if (make_transitive)
    something_changed=1;
    while (something_changed) %brute force loop until nothing changes. Could take a while, but don't expect K to be very large
        something_changed=0;
        for k1=1:K
            for k2=1:K
                for k3=1:K
                    if ((merge_matrix(k1,k2))&&(merge_matrix(k2,k3))&&(~merge_matrix(k1,k3)))
                        merge_matrix(k1,k3)=1;
                        something_changed=1;
                    end;
                end;
            end;
        end;
    end;
end

end

function test_get_mv_merge_matrix
mv_file=loadjson('example.mv');
mm=get_mv_merge_matrix(mv_file,[1,6,7,12,14],1);
disp(mm);
end