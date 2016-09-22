#include "mv_discrimhist_guide2.h"

#include "diskreadmda.h"
#include "extract_clips.h"
#include "mlcommon.h"
#include "compute_templates_0.h"
#include "msmisc.h"
#include "mv_discrimhist.h"
#include "cluster_scores.h"

struct discrimhist_guide2_data {
    int k1 = 0, k2 = 0;
    QVector<double> data1;
    QVector<double> data2;
    double overlap_score = 0;
};

struct discrimhist_guide2_data_comparer {
    bool operator()(const discrimhist_guide2_data& a, const discrimhist_guide2_data& b) const
    {
        return (a.overlap_score > b.overlap_score);
    }
};

/// TODO parallelize
bool mv_discrimhist_guide2(QString timeseries_path, QString firings_path, QString output_path, mv_discrimhist_guide2_opts opts)
{
    DiskReadMda32 X(timeseries_path);
    DiskReadMda F(firings_path);

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

    if (opts.cluster_numbers.isEmpty()) {
        for (int k = 1; k <= K; k++) {
            opts.cluster_numbers << k;
        }
    }

    QList<discrimhist_guide2_data> datas;

    QList<int> k1s, k2s;
    cluster_scores_opts opts00;
    opts00.add_noise_level = opts.add_noise_level;
    opts00.clip_size = opts.clip_size;
    opts00.cluster_numbers = opts.cluster_numbers;
    opts00.cluster_scores_only = 0;
    opts00.detect_threshold = 0;
    opts00.max_comparisons_per_cluster = opts.max_comparisons_per_cluster;
    ClusterScores::find_pairs_to_compare(k1s, k2s, X, F, opts00);
    for (int ii = 0; ii < k1s.count(); ii++) {
        int k1 = k1s[ii];
        int k2 = k2s[ii];
        printf("k1/k2 = %d/%d\n", k1, k2);

        discrimhist_guide2_data DD;
        DD.k1 = k1;
        DD.k2 = k2;

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

        QVector<double> scores_k1_k2 = ClusterScores::compute_cluster_pair_scores(X, clips_k1, clips_k2, opts00, &DD.data1, &DD.data2);
        DD.overlap_score = scores_k1_k2.value(0);
        datas << DD;
    }

    qSort(datas.begin(), datas.end(), discrimhist_guide2_data_comparer());

    long total_count = 0;
    for (int i = 0; i < datas.count(); i++) {
        total_count += datas[i].data1.count();
        total_count += datas[i].data2.count();
    }

    Mda output(4, total_count);
    //first two rows are k1/k2, third row is k1 or k2, fourth row is the value
    long jj = 0;
    for (int i = 0; i < datas.count(); i++) {
        int k1 = datas[i].k1;
        int k2 = datas[i].k2;
        for (long k = 0; k < datas[i].data1.count(); k++) {
            output.setValue(k1, 0, jj);
            output.setValue(k2, 1, jj);
            output.setValue(k1, 2, jj);
            output.setValue(datas[i].data1[k], 3, jj);
            jj++;
        }
        for (long k = 0; k < datas[i].data2.count(); k++) {
            output.setValue(k1, 0, jj);
            output.setValue(k2, 1, jj);
            output.setValue(k2, 2, jj);
            output.setValue(datas[i].data2[k], 3, jj);
            jj++;
        }
    }

    output.write32(output_path);

    return true;
}
