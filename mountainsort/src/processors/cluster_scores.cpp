/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 9/13/2016
*******************************************************/

#include "cluster_scores.h"

#include <diskreadmda.h>
#include <diskreadmda32.h>
#include "mlcommon.h"
#include "extract_clips.h"
#include "synthesize1.h" //for randn
#include "msmisc.h"
#include "compute_templates_0.h"
#include "jsvm.h"

namespace ClusterScores {
QVector<double> compute_cluster_scores(const DiskReadMda32& timeseries, const Mda32& clips, cluster_scores_opts opts);
double compute_distance(long N, float* ptr1, float* ptr2);
bool is_zero(const Mda& X);

bool cluster_scores(QString timeseries, QString firings, QString cluster_scores_path, QString cluster_pair_scores_path, cluster_scores_opts opts)
{
    DiskReadMda32 X(timeseries);
    DiskReadMda F(firings);

    QVector<double> times;
    QVector<int> labels;
    for (long i = 0; i < F.N2(); i++) {
        times << F.value(1, i);
        labels << (int)F.value(2, i);
    }
    int K = MLCompute::max(labels);
    if (opts.cluster_numbers.isEmpty()) {
        for (int k = 1; k <= K; k++) {
            opts.cluster_numbers << k;
        }
    }

    Mda cluster_scores(3, opts.cluster_numbers.count());
    for (int i = 0; i < opts.cluster_numbers.count(); i++) {
        int k = opts.cluster_numbers[i];
        printf("k=%d\n", k);
        QVector<double> times_k;
        for (long j = 0; j < labels.count(); j++) {
            if (labels[j] == k)
                times_k << times[j];
        }
        Mda32 clips_k = extract_clips(X, times_k, opts.clip_size);

        QVector<double> scores_k = compute_cluster_scores(X, clips_k, opts);
        cluster_scores.setValue(k, 0, i);
        for (int a = 0; a < scores_k.count(); a++) {
            cluster_scores.setValue(scores_k[a], a + 1, i);
        }
    }

    Mda cluster_pair_scores;
    if (!opts.cluster_scores_only) {
        QList<int> k1s, k2s;
        find_pairs_to_compare(k1s, k2s, X, F, opts);
        cluster_pair_scores.allocate(3, k1s.count());
        for (int ii = 0; ii < k1s.count(); ii++) {
            int k1 = k1s[ii];
            int k2 = k2s[ii];
            printf("k1/k2 = %d/%d\n", k1, k2);

            QVector<double> times_k1;
            for (long j = 0; j < labels.count(); j++) {
                if (labels[j] == k1)
                    times_k1 << times[j];
            }
            Mda32 clips_k1 = extract_clips(X, times_k1, opts.clip_size);

            QVector<double> times_k2;
            for (long j = 0; j < labels.count(); j++) {
                if (labels[j] == k2)
                    times_k2 << times[j];
            }
            Mda32 clips_k2 = extract_clips(X, times_k2, opts.clip_size);

            QVector<double> scores_k1_k2 = compute_cluster_pair_scores(X, clips_k1, clips_k2, opts);
            cluster_pair_scores.setValue(k1, 0, ii);
            cluster_pair_scores.setValue(k2, 1, ii);
            for (int a = 0; a < scores_k1_k2.count(); a++) {
                cluster_pair_scores.setValue(scores_k1_k2[a], a + 2, ii);
            }
        }
    }

    if (!cluster_scores.write64(cluster_scores_path))
        return false;

    if (!cluster_pair_scores.write64(cluster_pair_scores_path))
        return false;

    return true;
}

Mda32 add_self_noise_to_clips(const DiskReadMda32& timeseries, const Mda32& clips, double noise_factor)
{
    Mda32 ret = clips;
    int M = clips.N1();
    int T = clips.N2();
    QList<long> rand_inds;
    for (long i = 0; i < clips.N3(); i++) {
        rand_inds << T + (qrand() % timeseries.N2() - T * 2);
    }
    qSort(rand_inds); //good to do this so we don't random access the timeseries too much
    //could this add a bad bias?? don't really know.
    //one way around this is to randomize the order these are applied to the clips
    for (long i = 0; i < clips.N3(); i++) {
        long ind0 = rand_inds[i];
        Mda32 chunk;
        timeseries.readChunk(chunk, 0, ind0, M, T);
        for (int t = 0; t < T; t++) {
            for (int m = 0; m < M; m++) {
                ret.set(ret.get(m, t, i) + chunk.value(m, t) * noise_factor, m, t, i);
            }
        }
    }
    return ret;
}

Mda32 add_noise_to(const Mda32& X, double noise_level)
{
    Mda32 noise(X.N1(), X.N2(), X.N3());
    generate_randn(noise.totalSize(), noise.dataPtr());

    Mda32 ret(X.N1(), X.N2(), X.N3());
    for (long i = 0; i < ret.totalSize(); i++) {
        ret.set(noise.get(i) * noise_level + X.value(i), i);
    }
    return ret;
}

QVector<double> compute_cluster_scores(const DiskReadMda32& timeseries, const Mda32& clips, cluster_scores_opts opts)
{
    if (clips.N3() == 0)
        return QVector<double>();
    Mda32 template0 = compute_mean_clip(clips);
    int ind_m_of_peak = 0;
    int ind_t_of_peak = 0;
    int sign = 1;
    for (int t = 0; t < template0.N2(); t++) {
        for (int m = 0; m < template0.N1(); m++) {
            if (qAbs(template0.value(m, t)) > qAbs(template0.value(ind_m_of_peak, ind_t_of_peak))) {
                ind_m_of_peak = m;
                ind_t_of_peak = t;
                if (template0.value(m, t) > 0)
                    sign = 1;
                else
                    sign = -1;
            }
        }
    }
    //Mda32 clips_noise = add_noise_to(clips, opts.add_noise_level);
    Mda32 clips_noise = add_self_noise_to_clips(timeseries, clips, opts.add_noise_level);
    long num_err = 0;
    for (long i = 0; i < clips_noise.N3(); i++) {
        double val = sign * clips_noise.value(ind_m_of_peak, ind_t_of_peak, i);
        if (val < opts.detect_threshold)
            num_err++;
    }
    QVector<double> ret;
    ret << num_err * 1.0 / clips.N3();
    printf("%ld/%ld (%g%%)\n", num_err, clips.N3(), num_err * 1.0 / clips.N3() * 100);
    return ret;
}

double compute_distance(long N, float* ptr1, float* ptr2)
{
    double sumsqr = 0;
    for (long i = 0; i < N; i++) {
        double tmp = ptr1[i] - ptr2[i];
        sumsqr += tmp * tmp;
    }
    return sqrt(sumsqr);
}

bool is_zero(const Mda32& X)
{
    return ((X.minimum() == 0) && (X.maximum() == 0));
}

void find_pairs_to_compare(QList<int>& k1s, QList<int>& k2s, const DiskReadMda32& timeseries, const DiskReadMda& firings, cluster_scores_opts opts)
{
    k1s.clear();
    k2s.clear();

    QVector<double> times;
    QVector<int> labels;
    for (long i = 0; i < firings.N2(); i++) {
        times << firings.value(1, i);
        labels << (int)firings.value(2, i);
    }
    Mda32 templates = compute_templates_0(timeseries, times, labels, opts.clip_size);
    int M = templates.N1();
    int T = templates.N2();
    int K = templates.N3();

    for (int k1 = 1; k1 <= K; k1++) {
        if ((opts.cluster_numbers.isEmpty()) || (opts.cluster_numbers.contains(k1))) {
            QVector<double> dists;

            Mda32 template1;
            templates.getChunk(template1, 0, 0, k1 - 1, M, T, 1);

            QList<int> k2s_considered;
            for (int k2 = k1 + 1; k2 <= K; k2++) {
                if ((opts.cluster_numbers.isEmpty()) || (opts.cluster_numbers.contains(k2))) {
                    Mda32 template2;
                    templates.getChunk(template1, 0, 0, k2 - 1, M, T, 1);
                    double dist = compute_distance(M * T, template1.dataPtr(), template2.dataPtr());
                    if ((is_zero(template1)) || (is_zero(template2))) {
                        dists << 0;
                    }
                    else {
                        dists << dist;
                    }
                    k2s_considered << k2;
                }
            }

            QVector<double> dists_sorted = dists;
            qSort(dists_sorted);
            int num = qMax(opts.max_comparisons_per_cluster - 1, dists_sorted.count() - 1);
            double cutoff = dists_sorted[num];
            int ct = 0;
            for (int i2 = 0; i2 < dists.count(); i2++) {
                if ((dists[i2] <= cutoff) && (ct < opts.max_comparisons_per_cluster)) {
                    k1s << k1;
                    k2s << k2s_considered[i2];
                    ct++;
                }
            }
        }
    }
}
QVector<double> compute_cluster_pair_scores(DiskReadMda32 timeseries, const Mda32& clips1, const Mda32& clips2, cluster_scores_opts opts, QVector<double>* out_proj_data1, QVector<double>* out_proj_data2)
{
    long L1 = clips1.N3();
    long L2 = clips2.N3();
    if (!L1)
        return QVector<double>();
    if (!L2)
        return QVector<double>();
    int M = clips1.N1();
    int T = clips1.N2();

    qDebug() << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << opts.add_noise_level;
    Mda32 clips1_noise = add_self_noise_to_clips(timeseries, clips1, opts.add_noise_level);
    Mda32 clips2_noise = add_self_noise_to_clips(timeseries, clips2, opts.add_noise_level);

    Mda32 X(M * T, L1 + L2);
    QVector<int> labels;
    for (long i = 0; i < L1; i++) {
        for (int t = 0; t < T; t++) {
            for (int m = 0; m < M; m++) {
                X.setValue(clips1.value(m, t, i), m + M * t, i);
            }
        }
        labels << 1;
    }
    for (long i = 0; i < L2; i++) {
        for (int t = 0; t < T; t++) {
            for (int m = 0; m < M; m++) {
                X.setValue(clips2.value(m, t, i), m + M * t, i + L1);
            }
        }
        labels << 2;
    }

    double cutoff_00;
    QVector<double> direction_00;
    get_svm_discrim_direction(cutoff_00, direction_00, X, labels);

    long num_err = 0;
    for (long i = 0; i < L1; i++) {
        {
            QVector<double> tmp;
            for (int t = 0; t < T; t++) {
                for (int m = 0; m < M; m++) {
                    tmp << clips1_noise.value(m, t, i);
                }
            }
            double val = MLCompute::dotProduct(direction_00, tmp);
            if (val < cutoff_00)
                num_err++;
        }
        if (out_proj_data1) {
            QVector<double> out_tmp;
            for (int t = 0; t < T; t++) {
                for (int m = 0; m < M; m++) {
                    out_tmp << clips1.value(m, t, i);
                }
            }
            double val = MLCompute::dotProduct(direction_00, out_tmp);
            *out_proj_data1 << val;
        }
    }
    for (long i = 0; i < L2; i++) {
        {
            QVector<double> tmp;
            for (int t = 0; t < T; t++) {
                for (int m = 0; m < M; m++) {
                    tmp << clips2_noise.value(m, t, i);
                }
            }
            double val = MLCompute::dotProduct(direction_00, tmp);
            if (val > cutoff_00)
                num_err++;
        }
        if (out_proj_data2) {
            QVector<double> out_tmp;
            for (int t = 0; t < T; t++) {
                for (int m = 0; m < M; m++) {
                    out_tmp << clips2.value(m, t, i);
                }
            }
            double val = MLCompute::dotProduct(direction_00, out_tmp);
            *out_proj_data2 << val;
        }
    }

    printf("%ld/%ld (%g%%)    cutoff=%g\n", num_err, L1 + L2, num_err * 1.0 / (L1 + L2) * 100, cutoff_00);
    QVector<double> ret;
    ret << num_err * 1.0 / (L1 + L2);
    return ret;
}
}
