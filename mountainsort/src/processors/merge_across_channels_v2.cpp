/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/22/2016
*******************************************************/

#include "merge_across_channels_v2.h"
#include "diskreadmda.h"
#include "mlcommon.h"
#include "mlcommon.h"
#include "compute_templates_0.h"
#include "fit_stage.h"
#include "isosplit2.h"
#include <math.h>
#include "get_sort_indices.h"

bool peaks_are_within_range_to_consider(double p1, double p2);
bool cluster_is_already_being_used(const QVector<double>& times, const QVector<double>& other_times);

bool merge_across_channels_v2(const QString& timeseries_path, const QString& firings_path, const QString& firings_out_path, const merge_across_channels_v2_opts& opts)
{
    //Read the input arrays
    DiskReadMda X(timeseries_path);
    Mda firingsA;
    firingsA.read(firings_path);

    //sort the firings by time
    Mda firings = sort_firings_by_time(firingsA);

    int M = X.N1();
    int T = opts.clip_size;
    long L = firings.N2();

    //setup arrays for peakchans, times, labels (the info from firings.mda)
    QVector<int> peakchans;
    QVector<double> times;
    QVector<int> labels;
    for (long j = 0; j < L; j++) {
        peakchans << (int)firings.value(0, j);
        times << firings.value(1, j);
        labels << (int)firings.value(2, j);
    }
    int K = MLCompute::max<int>(labels);

    //compute the average waveforms (aka templates)
    Mda templates = compute_templates_0(X, times, labels, opts.clip_size);

    Mda channel_peaks(M, K);
    for (int k = 0; k < K; k++) {
        for (int m = 0; m < M; m++) {
            double peak_value = 0;
            for (int t = 0; t < T; t++) {
                double val = templates.value(m, t, k);
                if (qAbs(val) > qAbs(peak_value)) {
                    peak_value = val;
                }
            }
            channel_peaks.setValue(peak_value, m, k);
        }
    }

    //find the candidate pairs for merging
    Mda candidate_pairs(K, K);
    for (int k1 = 0; k1 < K; k1++) {
        QVector<long> inds1 = find_label_inds(labels, k1 + 1);
        if (!inds1.isEmpty()) {

            for (int k2 = 0; k2 < K; k2++) {
                QVector<long> inds2 = find_label_inds(labels, k2 + 1);
                if (!inds2.isEmpty()) {
                    int peakchan1 = peakchans[inds1[0]]; //the peak channel should be the same for all events with this labels, so we just need to look at the first one
                    int peakchan2 = peakchans[inds2[0]];
                    if (peakchan1 != peakchan2) { //only attempt to merge if the peak channels are different -- that's why it's called "merge_across_channels"
                        double val11 = channel_peaks.value(peakchan1 - 1, k1);
                        double val12 = channel_peaks.value(peakchan2 - 1, k1);
                        double val21 = channel_peaks.value(peakchan1 - 1, k2);
                        double val22 = channel_peaks.value(peakchan2 - 1, k2);
                        if ((peaks_are_within_range_to_consider(val11, val21)) && (peaks_are_within_range_to_consider(val12, val22))) {
                            candidate_pairs.setValue(k1, k2);
                        }
                    }
                }
            }
        }
    }

    //sort by largest peak so we can go through in order
    QVector<double> abs_peaks_on_own_channels;
    for (int k = 0; k < K; k++) {
        abs_peaks_on_own_channels << channel_peaks.value(peakchans[k] - 1, k);
    }
    QList<long> inds1 = get_sort_indices(abs_peaks_on_own_channels);

    QList<bool> clusters_to_use;
    for (int k = 0; k < K; k++)
        clusters_to_use << false;
    for (int ii = 0; ii < inds1.count(); ii++) {
        int ik = inds1[ii];
        QVector<long> inds_k = find_label_inds(labels, ik + 1);
        QVector<double> times_k;
        for (long a = 0; a < inds_k.count(); a++) {
            times_k << times[inds_k[a]];
        }
        QVector<double> other_times;
        for (int ik2 = 0; ik2 < K; ik2++) {
            if (candidate_pairs.value(ik, ik2)) {
                if (clusters_to_use[ik2]) { //we are using the other one
                    QVector<long> inds_k2 = find_label_inds(labels, ik2 + 1);
                    for (long a = 0; a < inds_k2.count(); a++) {
                        other_times << times[inds_k2[a]];
                    }
                }
            }
        }
        if (cluster_is_already_being_used(times_k, other_times)) {
            clusters_to_use[ik] = false;
        }
        else {
            clusters_to_use[ik] = true;
        }
    }

    //now we eliminate the clusters not to use
    QVector<long> inds_to_use;
    for (long ii = 0; ii < L; ii++) {
        int ik = labels[ii] - 1;
        if (clusters_to_use[ik]) {
            inds_to_use << ii;
        }
    }

    //set the output
    Mda firings_out(firings.N1(), inds_to_use.count());
    for (long i = 0; i < inds_to_use.count(); i++) {
        for (int j = 0; j < firings.N1(); j++) {
            firings_out.setValue(firings.value(j, inds_to_use[i]), j, i);
        }
    }

    printf("Using %ld of %ld events\n", firings_out.N2(), firings.N2());

    firings_out.write64(firings_out_path);

    return true;
}

bool peaks_are_within_range_to_consider(double p1, double p2)
{
    if ((!p1) || (!p2))
        return false;
    double ratio = p1 / p2;
    if (ratio > 1)
        ratio = 1 / ratio;
    return (ratio > 0.7); //arbitrary choice 0.7
}

bool cluster_is_already_being_used(const QVector<double>& times, const QVector<double>& other_times)
{
    //FINISH!!!
}
