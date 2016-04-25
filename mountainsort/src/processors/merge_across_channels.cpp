/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/22/2016
*******************************************************/

#include "merge_across_channels.h"
#include "diskreadmda.h"
#include "msmisc.h"
#include "compute_templates_0.h"
#include "fit_stage.h"
#include "isosplit2.h"

void compute_merge_score(double& score0, double best_dt0, Mda& template1, Mda& template2, QList<double>& times1, QList<double>& times2, double peakchan1, double peakchan2, const merge_across_channels_opts& opts);

bool merge_across_channels(const QString& timeseries_path, const QString& firings_path, const QString& firings_out_path, const merge_across_channels_opts& opts)
{
    DiskReadMda X(timeseries_path);
    Mda firingsA;
    firingsA.read(firings_path);

    //sort the firings by time
    Mda firings = sort_firings_by_time(firingsA);

    int M = X.N1();
    int T = opts.clip_size;
    long L = firings.N2();

    //setup arrays for peakchans, times, labels
    QList<int> peakchans;
    QList<double> times;
    QList<int> labels;
    for (long j = 0; j < L; j++) {
        peakchans << (int)firings.value(0, j);
        times << firings.value(1, j);
        labels << (int)firings.value(2, j);
    }
    int K = compute_max(labels);

    Mda templates = compute_templates_0(X, times, labels, opts.clip_size);

    //assemble the matrices S and best_dt
    //for now S is binary/boolean
    Mda S(K, K);
    Mda best_dt(K, K);
    for (int k1 = 1; k1 <= K; k1++) {
        for (int k2 = 1; k2 <= K; k2++) {
            QList<long> inds1 = find_inds(labels, k1);
            QList<long> inds2 = find_inds(labels, k2);
            if ((!inds1.isEmpty()) && (!inds2.isEmpty())) {
                Mda template1, template2;
                templates.getChunk(template1, 0, 0, k1 - 1, M, T, 1);
                templates.getChunk(template2, 0, 0, k2 - 1, M, T, 1);
                int peakchan1 = peakchans[inds1[0]];
                int peakchan2 = peakchans[inds2[0]];
                double score0, best_dt0;
                QList<double> times1, times2;
                for (long a = 0; a < inds1.count(); a++)
                    times1 << times[inds1[a]];
                for (long a = 0; a < inds2.count(); a++)
                    times2 << times[inds2[a]];
                compute_merge_score(score0, best_dt0, template1, template2, times1, times2, peakchan1, peakchan2, opts);
                S.setValue(score0, k1 - 1, k2 - 1);
                best_dt.setValue(best_dt0, k1 - 1, k2 - 1);
            }
        }
    }

    //finish!!

    return true;
}

//////Oh boy, check sign!!!!!!!!!!!
QList<double> compute_cross_correlograms(const QList<double> &times1_in,const QList<double> &times2_in,int bin_min,int bin_max) {
    QList<double> times1=times1_in;
    QList<double> times2=times2_in;

    qSort(times1);
    qSort(times2);

    QList<double> ret;
    for (int i=bin_min; i<=bin_max; i++) ret << 0;

    long j=0;
    for (long i=0; i<times1.count(); i++) {
        while (times2[j]>=times1[i]+bin_min) j++;
        while (times2[j]<=times1[i]+bin_max) {
            double diff=times2[j]-times1[i];
            long diff0=(long)(diff+0.5);
            if ((diff0>=bin_min)&&(diff0<=bin_max)) {
                ret[diff0-bin_min]++;
            }
        }
    }

    return ret;
}

void compute_merge_score(double& score0, double best_dt0, Mda& template1, Mda& template2, QList<double>& times1, QList<double>& times2, double peakchan1, double peakchan2, const merge_across_channels_opts& opts)
{
    int M=template1.N1();
    int T=template1.N2();

    //values to return if no merge
    score0 = 0;
    bestdt = 0;

    //min_peak_ratio criterion
    double template1_self_peak = max_absolute_value_on_channel(template1, peakchan1);
    double template1_other_peak = max_absolute_value_on_channel(template1, peakchan2);
    if (template1_other_peak < opts.min_peak_ratio*template1_self_peak) {
        return;
    }

    //min_template_corr_coef criterion
    double r12=compute_noncentered_correlation(M*T,template1.dataPtr(),template2.dataPtr());
    if (r12<opts.min_template_corr_coef) {
        return;
    }

    //check if one of the time lists is empty
    if ((times1.isEmpty())||(times2.isEmpty())) {
        return;
    }

    //compute firing cross-correlogram
    QList<double> bin_counts=compute_cross_correlogram(times1,times2,-opts.max_dt,opts.max_dt);
    double sum_bin_counts=compute_sum(bins);
    if (!sum_bin_counts) return;

    //compute mean offset
    double mean_dt=0;
    for (int dt=-opts.max_dt; i<=opts.max_dt; i++) {
        mean_dt+=dt*bin_counts[dt+opts.max_dt];
    }
    mean_dt/=sum_bin_counts;

    //compute stdev of offsets
    double stdev_dt=0;
    for (int dt=-opts.max_dt; i<=opts.max_dt; i++) {
        stdev_dt+=(dt-mean_dt)*(dt-mean_dt)*bin_counts[dt+opts.max_dt];
    }
    stdev_dt=sqrt(stdev_dt/sum_bin_counts);

    //min_coinc_frac, min_coinc_num, and max_corr_stdev criterion
    double coincfrac=sum0/qMin(times1.count(),times2.count());
    if ((coincfrac>opts.min_coinc_frac)&&(sum0>=opts.min_coinc_num)&&(stdev_dt<opts.max_corr_stdev)) {
        best_dt0=mean_dt;
        score0=1;
    }
}
