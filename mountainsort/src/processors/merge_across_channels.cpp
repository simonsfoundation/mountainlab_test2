/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/22/2016
*******************************************************/

#include "merge_across_channels.h"
#include "diskreadmda.h"
#include "mlcommon.h"
#include "mlcommon.h"
#include "compute_templates_0.h"
#include "fit_stage.h"
#include "isosplit2.h"
#include <math.h>

void compute_merge_score(double& score0, double& best_dt0, Mda& template1, Mda& template2, QVector<double>& times1, QVector<double>& times2, double peakchan1, double peakchan2, const merge_across_channels_opts& opts);
void make_reflexive_and_transitive(Mda& S, Mda& best_dt);
Mda remove_redundant_events(Mda& firings, int maxdt);
QVector<int> remove_unused_labels(const QVector<int>& labels);

bool merge_across_channels(const QString& timeseries_path, const QString& firings_path, const QString& firings_out_path, const merge_across_channels_opts& opts)
{
    DiskReadMda32 X(timeseries_path);
    Mda64 firingsA;
    firingsA.read(firings_path);

    //sort the firings by time
    Mda64 firings = sort_firings_by_time(firingsA);

    int M = X.N1();
    int T = opts.clip_size;
    long L = firings.N2();

    //setup arrays for peakchans, times, labels
    QVector<int> peakchans;
    QVector<double> times;
    QVector<int> labels;
    for (long j = 0; j < L; j++) {
        peakchans << (int)firings.value(0, j);
        times << firings.value(1, j);
        labels << (int)firings.value(2, j);
    }
    int K = MLCompute::max<int>(labels);

    Mda32 templates = compute_templates_0(X, times, labels, opts.clip_size);

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
                QVector<double> times1, times2;
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
    S.write32("/tmp/debugS.mda");
    best_dt.write32("/tmp/debugbest_dt.mda");

    //make the matrix reflexive and transitive
    make_reflexive_and_transitive(S, best_dt);

    //now we merge based on the above scores
    QVector<int> new_labels = labels;
    QVector<double> new_times = times;
    for (int k1 = 1; k1 <= K; k1++) {
        for (int k2 = 1; k2 <= K; k2++) {
            if (S.value(k1 - 1, k2 - 1)) {
                //now we merge
                QList<long> inds_k2 = find_inds(new_labels, k2);
                for (long j = 0; j < inds_k2.count(); j++) {
                    new_labels[inds_k2[j]] = k1;
                    new_times[inds_k2[j]] = times[inds_k2[j]] - best_dt.value(k1 - 1, k2 - 1);
                }
            }
        }
    }

    //remove unused labels
    new_labels = remove_unused_labels(new_labels);
    int K_new = MLCompute::max<int>(new_labels);
    printf("Merged into %d of %d clusters\n", K_new, K);

    //set the output
    Mda firings_out = firings;
    for (long i = 0; i < firings_out.N2(); i++) {
        firings_out.setValue(new_times[i], 1, i);
        firings_out.setValue(new_labels[i], 2, i);
    }

    //Now we will have a bunch of redundant events! So let's remove them!
    long maxdt = 5;
    firings_out = remove_redundant_events(firings_out, maxdt);

    printf("Using %ld of %ld events\n", firings_out.N2(), firings.N2());

    firings_out.write64(firings_out_path);

    return true;
}

void make_reflexive_and_transitive(Mda& S, Mda& best_dt)
{
    int K = S.N1();
    bool something_changed = true;
    while (something_changed) {
        something_changed = false;
        //reflexive
        for (int k1 = 0; k1 < K; k1++) {
            for (int k2 = 0; k2 < K; k2++) {
                if (S.value(k1, k2) && (!S.value(k2, k1))) {
                    something_changed = true;
                    S.setValue(S.value(k1, k2), k2, k1);
                    best_dt.setValue(-best_dt.value(k1, k2), k2, k1);
                }
            }
        }
        //transitive
        for (int k1 = 0; k1 < K; k1++) {
            for (int k2 = 0; k2 < K; k2++) {
                for (int k3 = 0; k3 < K; k3++) {
                    if (S.value(k1, k2) && (S.value(k2, k3)) && (!S.value(k1, k3))) {
                        something_changed = true;
                        S.setValue((S.value(k1, k2) + S.value(k2, k3)) / 2, k1, k3);
                        best_dt.setValue(best_dt.value(k1, k2) + best_dt.value(k2, k3), k1, k3);
                    }
                }
            }
        }
    }
}

//////Oh boy, very important, check sign!!!!!!!!!!!
QVector<double> compute_cross_correlogram(const QVector<double>& times1_in, const QVector<double>& times2_in, int bin_min, int bin_max)
{
    QVector<double> times1 = times1_in;
    QVector<double> times2 = times2_in;

    qSort(times1);
    qSort(times2);

    QVector<double> ret;
    for (int i = bin_min; i <= bin_max; i++)
        ret << 0;

    long j = 0;
    for (long i = 0; i < times1.count(); i++) {
        while ((j >= 0) && (times2[j] >= times1[i] + bin_min))
            j--;
        while ((j < times2.count()) && (times2[j] <= times1[i] + bin_max)) {
            double diff = times1[i] - times2[j];
            long diff0 = (long)(diff + 0.5);
            if ((diff0 >= bin_min) && (diff0 <= bin_max)) {
                ret[diff0 - bin_min]++;
            }
            j++;
        }
    }

    return ret;
}

double max_absolute_value_on_channel(Mda& template1, int channel)
{
    QVector<double> vals;
    for (int i = 0; i < template1.N2(); i++) {
        vals << qAbs(template1.value(channel - 1, i));
    }
    return MLCompute::max(vals);
}

double compute_noncentered_correlation(long N, double* X1, double* X2)
{
    double S12 = 0, S11 = 0, S22 = 0;
    for (long i = 0; i < N; i++) {
        S12 += X1[i] * X2[i];
        S11 += X1[i] * X1[i];
        S22 += X2[i] * X2[i];
    }
    if ((!S11) || (!S22))
        return 0;
    return S12 / (sqrt(S11) * sqrt(S22));
}

Mda time_shift_template(Mda& template0, int dt)
{
    int M = template0.N1();
    int T = template0.N2();
    Mda ret(M, T);
    for (int t = 0; t < T; t++) {
        for (int m = 0; m < M; m++) {
            ret.setValue(template0.value(m, t - dt), m, t);
        }
    }
    return ret;
}

double compute_sliding_noncentered_correlation(Mda& template1, Mda& template2)
{
    int M = template1.N1();
    int T = template1.N2();
    double best = 0;
    for (int dt = -T / 2; dt <= T / 2; dt++) {
        Mda template1b = time_shift_template(template1, dt);
        double val = compute_noncentered_correlation(M * T, template1b.dataPtr(), template2.dataPtr());
        if (val > best)
            best = val;
    }
    return best;
}

double compute_sum(const QVector<double>& X)
{
    double ret = 0;
    for (long i = 0; i < X.count(); i++)
        ret += X[i];
    return ret;
}

void compute_merge_score(double& score0, double& best_dt0, Mda& template1, Mda& template2, QVector<double>& times1, QVector<double>& times2, double peakchan1, double peakchan2, const merge_across_channels_opts& opts)
{
    //values to return if no merge
    score0 = 0;
    best_dt0 = 0;

    //min_peak_ratio criterion
    double template1_self_peak = max_absolute_value_on_channel(template1, peakchan1);
    double template1_other_peak = max_absolute_value_on_channel(template1, peakchan2);
    if (template1_other_peak < opts.min_peak_ratio * template1_self_peak) {
        return;
    }

    //min_template_corr_coef criterion
    //double r12 = compute_noncentered_correlation(M * T, template1.dataPtr(), template2.dataPtr());
    double r12 = compute_sliding_noncentered_correlation(template1, template2);
    if (r12 < opts.min_template_corr_coef) {
        return;
    }

    //check if one of the time lists is empty
    if ((times1.isEmpty()) || (times2.isEmpty())) {
        return;
    }

    //compute firing cross-correlogram
    QVector<double> bin_counts = compute_cross_correlogram(times1, times2, -opts.max_dt, opts.max_dt);
    double sum_bin_counts = compute_sum(bin_counts);
    if (!sum_bin_counts)
        return;

    //compute mean offset
    double mean_dt = 0;
    for (int dt = -opts.max_dt; dt <= opts.max_dt; dt++) {
        mean_dt += dt * bin_counts[dt + opts.max_dt];
    }
    mean_dt /= sum_bin_counts;

    //compute stddev of offsets
    double stddev_dt = 0;
    for (int dt = -opts.max_dt; dt <= opts.max_dt; dt++) {
        stddev_dt += (dt - mean_dt) * (dt - mean_dt) * bin_counts[dt + opts.max_dt];
    }
    stddev_dt = sqrt(stddev_dt / sum_bin_counts);

    //min_coinc_frac, min_coinc_num, and max_corr_stddev criterion
    double coincfrac = sum_bin_counts / qMin(times1.count(), times2.count());
    if ((coincfrac > opts.min_coinc_frac) && (sum_bin_counts >= opts.min_coinc_num) && (stddev_dt < opts.max_corr_stddev)) {
        best_dt0 = mean_dt;
        score0 = 1;
    }
}

Mda remove_redundant_events(Mda& firings, int maxdt)
{
    QVector<double> times;
    QVector<int> labels;

    long L = firings.N2();
    for (long i = 0; i < L; i++) {
        times << firings.value(1, i);
        labels << (int)firings.value(2, i);
    }
    int K = MLCompute::max<int>(labels);

    QVector<int> to_use;
    for (long i = 0; i < L; i++)
        to_use << 1;
    for (int k = 1; k <= K; k++) {
        QList<long> inds_k = find_inds(labels, k);
        QVector<double> times_k;
        for (long i = 0; i < inds_k.count(); i++)
            times_k << times[inds_k[i]];
        //the bad indices are those whose times occur too close to the previous times
        for (long i = 1; i < times_k.count(); i++) {
            if (times_k[i] <= times_k[i - 1] + maxdt)
                to_use[inds_k[i]] = 0;
        }
    }

    QList<long> inds_to_use;
    for (long i = 0; i < L; i++) {
        if (to_use[i])
            inds_to_use << i;
    }

    Mda firings_out(firings.N1(), inds_to_use.count());
    for (long i = 0; i < inds_to_use.count(); i++) {
        for (int j = 0; j < firings.N1(); j++) {
            firings_out.setValue(firings.value(j, inds_to_use[i]), j, i);
        }
    }

    return firings_out;
}

QVector<int> remove_unused_labels(const QVector<int>& labels)
{
    int K = MLCompute::max<int>(labels);
    QVector<int> used_labels;
    for (int k = 1; k <= K; k++)
        used_labels << 0;
    for (long i = 0; i < labels.count(); i++) {
        if (labels[i] > 0) {
            used_labels[labels[i] - 1] = 1;
        }
    }
    QVector<int> used_label_numbers;
    for (int k = 1; k <= K; k++) {
        if (used_labels[k - 1])
            used_label_numbers << k;
    }
    QVector<int> label_map;
    for (int k = 0; k <= K; k++)
        label_map << 0;
    for (int j = 0; j < used_label_numbers.count(); j++)
        label_map[used_label_numbers[j]] = j + 1;
    QVector<int> labels_out;
    for (long i = 0; i < labels.count(); i++) {
        labels_out << label_map[labels[i]];
    }
    return labels_out;
}
