function cluster_numbers=get_mv_clusters_with_tag(mv_file,tag)

cluster_numbers=[];

if (ischar(mv_file))
    mv_file=loadjson(mv_file);
end;

CA=mv_file.cluster_attributes;
names=fieldnames(CA);
for j=1:length(names)
    if contains_tag(CA.(names{j}).tags,tag)
        cluster_numbers=[cluster_numbers,get_cluster_number_from_field(names{j})];
    end;
end;

end

function ret=contains_tag(list,tag)
for j=1:length(list)
    if (strcmp(list{j},tag))
        ret=1;
        return;
    end;
end;
ret=0;
end

function num=get_cluster_number_from_field(field)
num=str2num(field(3:end));
end