function [firings,info]=alg_scda_005(timeseries_fname,output_dir,opts)
% ALG_SCDA_005  Matlab implementation of JFM's javascript/C++ sorter of July 2017
%
% [firingsfile,info] = alg_scda_005(rawfile,output_dir,o)
%
% Inputs:
%    rawfile - path to .mda of MxN (ie # channels by # timepoints) raw signal data
%    output_dir - path to existing directory where all output will be written
%    o - optional, contains sorting options, can include any of:
%                   clip_size=100;
%                   detect_threshold=4;
%                   detect_interval=10;
%                   shell_increment=3;
%                   min_shell_size=150;
%                   samplerate=30000;
%                   sign=0;
%                   freq_min=300;
%                   freq_max=10000;
%                   num_fea=10;
%                   adj_radius=0;
%                   channels=[];
%                   timerange=[-1,-1];
%                   use_whitening=1;
%                   use_mask_out_artifacts=1;
%                   geom=''; path of CSV file giving x,y coords of each electrode
%                            (if absent or empty assumes full dense connectivity)
%                   detectmeth = 0, 3, or 4 (controls detection method)
%                   neglogprior = 30;
% Outputs:
%    firingsfile - path to the firings.mda output file
%    info - struct with fields:
%           filtfile - filtered timeseries
%           prefile - path to the preprocessed timeseries (filt and whitened)
%
% For now detect3 uses pure-matlab version - todo: write processor adjust_times
%  to allow executable to do it.
%
% Also see: mountainlab_devel/sorting_algs/alg_scda_005_js.m  which is just a
%           wrapper to alg_scda_005.js

% Magland & Barnett 7/27/16
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
def_opts.detectmeth = 0;
def_opts.neglogprior = 30;
opts=ms_set_default_opts(opts,def_opts);

raw=timeseries_fname;
geom=opts.geom;

o_geom2adj=struct('channels',opts.channels,'radius',opts.adj_radius);
o_extract_raw=struct('t1',opts.timerange(1),'t2',opts.timerange(2),'channels',opts.channels);
o_filter=struct('samplerate',opts.samplerate,'freq_min',opts.freq_min,'freq_max',opts.freq_max);
o_mask_out_artifacts=struct('threshold',3,'interval_size',200);
o_whiten=struct;
o_detect=struct('detect_threshold',opts.detect_threshold,'detect_interval',opts.detect_interval,'clip_size',opts.clip_size,'sign',opts.sign,'individual_channels',1);
o_branch_cluster=struct('clip_size',opts.clip_size,'min_shell_size',opts.min_shell_size,'shell_increment',opts.shell_increment,'num_features',opts.num_fea,'detect_interval',opts.detect_interval,'consolidation_factor',0.9);
o_compute_outlier_scores=struct('clip_size',opts.clip_size,'shell_increment',opts.shell_increment,'min_shell_size',opts.min_shell_size);
o_compute_detectability_scores=struct('clip_size',opts.clip_size,'shell_increment',opts.shell_increment,'min_shell_size',opts.min_shell_size);
o_merge_across_channels=struct('min_peak_ratio',0.7,'max_dt',10,'min_coinc_frac',0.1,'min_coinc_num',10,'max_corr_stddev',3,'min_template_corr_coef',0.5,'clip_size',opts.clip_size);
o_fit_stage=struct('clip_size',opts.clip_size+6.,'min_shell_size',opts.min_shell_size,'shell_increment',opts.shell_increment,'neglogprior',opts.neglogprior);

pre0=[output_dir,'/pre0.mda'];
pre1=[output_dir,'/pre1.mda'];
pre1b=[output_dir,'/pre1b.mda'];
pre2=[output_dir,'/pre2.mda'];
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
if opts.detectmeth==0
  mscmd_detect(pre2,detect,o_detect);
elseif opts.detectmeth==3          % replace w/ mscmd_detect3 when ready:
  %o_detect.beta=5;
  fprintf('\n---- DETECT3 in MATLAB ----\n'); o_detect
  Y = readmda(pre2);          % for now use pure matlab...
  [times chans] = ms_detect3(Y,o_detect);
  writemda([chans;times], detect, 'float64');
elseif opts.detectmeth==4          % replace w/ mscmd_detect4 when ready ?
  fprintf('\n---- DETECT4 in MATLAB ----\n'); o_detect
  Y = readmda(pre2);          % for now use pure matlab...
  [times chans] = ms_detect4(Y,o_detect);
  writemda([chans;times], detect, 'float64');
else
  error('unknown opts.detectmeth');
end
%F=readmda(detect); F(2,:)=floor(+F(2,:)); writemda(F,detect,'float64'); % for debugging: round times
mscmd_branch_cluster_v2(pre2,detect,adjacency_matrix,firings1,o_branch_cluster);
mscmd_merge_across_channels(pre2,firings1,firings2,o_merge_across_channels);
mscmd_fit_stage(pre2,firings2,firings3,o_fit_stage);
mscmd_compute_outlier_scores(pre2,firings3,firings4,o_compute_outlier_scores);
mscmd_compute_detectability_scores(pre2,firings4,firings5,o_compute_detectability_scores);

mscmd_copy(firings5,firings);

info.filtfile = pre1b;      % send info out
info.prefile = pre2;
end