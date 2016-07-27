function [firings_fname,info]=alg_scda_005(timeseries_fname,output_dir,opts)

def_opts.clip_size=100;
def_opts.detect_threshold=4;
def_opts.detect_interval=10;
def_opts.shell_increment=3;
def_opts.min_shell_size=150;
def_opts.samplerate=30000;
def_opts.sign=0;
def_opts.freq_min=300;
def_opts.freq_max=10000;
def_opts.num_fea=10;
def_opts.adj_radius=0;
def_opts.channels=[];
def_opts.timerange=[-1,-1];
def_opts.use_whitening=1;
def_opts.use_mask_out_artifacts=1;
def_opts.geom='';
opts=ms_set_default_opts(opts,def_opts);

raw=timeseries;
geom=params.geom;

o_geom2adj=struct('channels',opts.channels,'radius',opts.adj_radius);
o_extract_raw=struct('t1',opts.timerange(1),'t2',opts.timerange(2)),'channels',opts.channels);
o_filter=struct('samplerate',opts.samplerate,'freq_min',opts.freq_min,'freq_max',opts.freq_max);
o_mask_out_artifacts=struct('threshold',3,'interval_size',200);
o_whiten=struct;
o_detect=struct('detect_threshold',opts.detect_threshold,'detect_interval',opts.detect_interval,'clip_size',opts.clip_size,'sign',opts.sign,'individual_channels',1);
o_branch_cluster=struct('clip_size',opts.clip_size,'min_shell_size',opts.min_shell_size,'shell_increment',opts.shell_increment,'num_features',opts.num_fea,'detect_interval',opts.detect_interval,'consolidation_factor',0.9);
o_compute_outlier_scores=struct('clip_size',opts.clip_size,'shell_increment',opts.shell_increment,'min_shell_size',opts.min_shell_size);
o_compute_detectability_scores=struct('clip_size',opts.clip_size,'shell_increment',opts.shell_increment,'min_shell_size',opts.min_shell_size);
o_merge_across_channels=struct('min_peak_ratio',0.7,'max_dt',10,'min_coinc_frac',0.1,'min_coinc_num',10,'max_corr_stddev',3,'min_template_corr_coef',0.5,'clip_size',opts.clip_size);
o_fit_stage=struct('clip_size',opts.clip_size+6.,'min_shell_size',opts.min_shell_size,'shell_increment',opts.shell_increment);

pre0=[output_dir,'/pre0.mda'];
pre1=[output_dir,'/pre1.mda'];
pre1b=[output_dir,'/pre1b.mda'];
detect=[output_dir,'/detect.mda'];
firings1=[output_dir,'/firings1.mda'];
firings2=[output_dir,'/firings2.mda'];
firings3=[output_dir,'/firings3.mda'];
firings4=[output_dir,'/firings4.mda'];
firings5=[output_dir,'/firings5.mda'];
firings=[output_dir,'/firings.mda'];

if (~isempty(geom))
    adjacency_matrix=[output_dir,'/AM.mda'];
    mscmd_geom2adj(geom,adjacency_matrix,o_geom2adj);
else
    adjacency_matrix='';
end;

mscmd_extract_raw(raw,pre0,o_extract_raw);
if (opts.use_mask_out_artifacts)
    mscmd_bandpass_filter(pre0,pre1,o_filter);
    mscmd_mask_out_artifacts(pre1,pre1b,o_mask_out_artifacts);
else
    mscmd_bandpass_filter(pre0,pre1b,o_filter);
end;
if (opts.use_whitening)
    mscmd_whiten(pre1b,pre2,o_whiten);
else
    mscmd_normalize_channels(pre1b,pre2,struct);
end;
mscmd_detect(pre2,detect,o_detect);
mscmd_branch_cluster_v2(pre2,detect,adjacency_matrix,firings1,o_branch_cluster);
mscmd_merge_across_channels(pre2,firings1,firings2,o_merge_across_channels);
mscmd_fit_stage(pre2,firings2,firings3,o_fit_stage);
mscmd_compute_outlier_scores(pre2,firings3,firings4,o_compute_outlier_scores);
mscmd_compute_detectability_scores(pre2,firings4,firings5,o_compute_detectability_scores);

mscmd_copy(firings5,firings);

end