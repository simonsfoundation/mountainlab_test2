/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 9/13/2016
*******************************************************/

#include "kdtree.h"
#include "ms_metrics.h"

#include <QTime>
#include <diskreadmda.h>
#include <diskreadmda32.h>
#include "mlcommon.h"
#include "extract_clips.h"
#include "synthesize1.h" //for randn
#include "msmisc.h"
#include "compute_templates_0.h"
#include "jsvm.h"
#include "noise_nearest.h"
#include "get_sort_indices.h"

namespace MSMetrics {

struct Metric {
    QList<double> values;
};

double compute_noise_overlap(const DiskReadMda32& X, const QVector<double>& times, ms_metrics_opts opts);
double compute_overlap(const DiskReadMda32& X, const QVector<double>& times1, const QVector<double>& times2, ms_metrics_opts opts);
QSet<QString> get_pairs_to_consider(const DiskReadMda32& X, const DiskReadMda& F, ms_metrics_opts opts);

bool ms_metrics(QString timeseries, QString firings, QString cluster_metrics_path, QString cluster_pair_metrics_path, ms_metrics_opts opts)
{
    DiskReadMda32 X(timeseries);
    DiskReadMda F(firings);

    //define opts.cluster_numbers in case it is empty
    QVector<int> labels0;
    for (long i = 0; i < F.N2(); i++) {
        labels0 << (int)F.value(2, i);
    }
    int K = MLCompute::max(labels0);
    if (opts.cluster_numbers.isEmpty()) {
        for (int k = 1; k <= K; k++) {
            opts.cluster_numbers << k;
        }
    }
    QSet<int> cluster_numbers_set;
    for (int i = 0; i < opts.cluster_numbers.count(); i++) {
        cluster_numbers_set.insert(opts.cluster_numbers[i]);
    }
    qDebug() << "Using cluster numbers:" << opts.cluster_numbers;

    printf("Extracting times and labels...\n");
    //QVector<long> inds;
    QVector<double> times;
    QVector<int> labels;
    for (long i = 0; i < F.N2(); i++) {
        int label0 = (int)F.value(2, i);
        if (cluster_numbers_set.contains(label0)) {
            //inds << i;
            times << F.value(1, i);
            labels << label0;
        }
    }

    /*
    noise_nearest_opts nn_opts;
    nn_opts.cluster_numbers = opts.cluster_numbers;
    nn_opts.clip_size = opts.clip_size;
    nn_opts.add_noise_level = opts.add_noise_level;
    Mda isolation_matrix = NoiseNearest::compute_isolation_matrix(timeseries, firings, nn_opts);
    */

    printf("Cluster metrics...\n");
    QMap<QString, Metric> cluster_metrics;
    QTime timer;
    timer.start();
    for (int i = 0; i < opts.cluster_numbers.count(); i++) {
        if (timer.elapsed() > 5000) {
            qDebug() << QString("Cluster %1 of %2").arg(i + 1).arg(opts.cluster_numbers.count());
            timer.restart();
        }
        int k = opts.cluster_numbers[i];
        QVector<double> times_k;
        for (long i = 0; i < times.count(); i++) {
            if (labels[i] == k) {
                times_k << times[i];
            }
        }
        Mda32 clips_k = extract_clips(X, times_k, opts.clip_size);
        Mda32 template_k = compute_mean_clip(clips_k);
        Mda32 stdev_k = compute_stdev_clip(clips_k);
        {
            double min0 = template_k.minimum();
            double max0 = template_k.maximum();
            cluster_metrics["peak_amp"].values << qMax(qAbs(min0), qAbs(max0));
        }
        {
            double min0 = stdev_k.minimum();
            double max0 = stdev_k.maximum();
            cluster_metrics["peak_noise"].values << qMax(qAbs(min0), qAbs(max0));
        }
        {
            cluster_metrics["noise_overlap"].values << compute_noise_overlap(X, times_k, opts);
        }
        /*
        {
            double numer = isolation_matrix.value(i, opts.cluster_numbers.count());
            double denom = times_k.count();
            if (!denom)
                denom = 1;
            double val = numer / denom;
            cluster_metrics["noise_overlap"].values << val;
        }
        {
            double numer = isolation_matrix.value(i, i);
            double denom = times_k.count();
            if (!denom)
                denom = 1;
            double val = numer / denom;
            cluster_metrics["isolation"].values << val;
        }
        */
    }

    QStringList cluster_metric_names = cluster_metrics.keys();
    QString cluster_metric_txt = "cluster," + cluster_metric_names.join(",") + "\n";
    for (int i = 0; i < opts.cluster_numbers.count(); i++) {
        QString line = QString("%1").arg(opts.cluster_numbers[i]);
        foreach (QString name, cluster_metric_names) {
            line += QString(",%1").arg(cluster_metrics[name].values[i]);
        }
        cluster_metric_txt += line + "\n";
    }
    if (!TextFile::write(cluster_metrics_path, cluster_metric_txt)) {
        qWarning() << "Problem writing output file: " + cluster_metrics_path;
        return false;
    }

    QSet<QString> pairs_to_consider = get_pairs_to_consider(X, F, opts);

    //////////////////////////////////////////////////////////////
    printf("Cluster pair metrics...\n");
    QMap<QString, Metric> cluster_pair_metrics;
    QTime timer1;
    timer1.start();
    for (int i1 = 0; i1 < opts.cluster_numbers.count(); i1++) {
        if (timer1.elapsed() > 5000) {
            qDebug() << QString("Cluster pair metrics %1 of %2").arg(i1 + 1).arg(opts.cluster_numbers.count());
            timer1.restart();
        }
        for (int i2 = 0; i2 < opts.cluster_numbers.count(); i2++) {
            int k1 = opts.cluster_numbers[i1];
            int k2 = opts.cluster_numbers[i2];
            if (pairs_to_consider.contains(QString("%1-%2").arg(k1).arg(k2))) {
                QVector<double> times_k1;
                for (long i = 0; i < times.count(); i++) {
                    if (labels[i] == k1) {
                        times_k1 << times[i];
                    }
                }
                QVector<double> times_k2;
                for (long i = 0; i < times.count(); i++) {
                    if (labels[i] == k2) {
                        times_k2 << times[i];
                    }
                }
                double val = compute_overlap(X, times_k1, times_k2, opts);
                if (val >= 0.01) { //to save some space in the file
                    cluster_pair_metrics["overlap"].values << val;
                }
                else {
                    cluster_pair_metrics["overlap"].values << 0;
                }
            }
            else {
                cluster_pair_metrics["overlap"].values << 0;
            }
        }
    }

    QStringList cluster_pair_metric_names = cluster_pair_metrics.keys();
    QString cluster_pair_metric_txt = "cluster1,cluster2," + cluster_pair_metric_names.join(",") + "\n";
    int jj = 0;
    for (int i1 = 0; i1 < opts.cluster_numbers.count(); i1++) {
        for (int i2 = 0; i2 < opts.cluster_numbers.count(); i2++) {
            bool has_something_non_zero = false;
            foreach (QString name, cluster_pair_metric_names) {
                if (cluster_pair_metrics[name].values[jj])
                    has_something_non_zero = true;
            }
            if ((has_something_non_zero) && (i1 != i2)) {
                QString line = QString("%1,%2").arg(opts.cluster_numbers[i1]).arg(opts.cluster_numbers[i2]);
                foreach (QString name, cluster_pair_metric_names) {
                    line += QString(",%1").arg(cluster_pair_metrics[name].values[jj]);
                }
                cluster_pair_metric_txt += line + "\n";
            }
            jj++;
        }
    }
    if (!TextFile::write(cluster_pair_metrics_path, cluster_pair_metric_txt)) {
        qWarning() << "Problem writing output file: " + cluster_pair_metrics_path;
        return false;
    }

    return true;
}

long random_time(long N, int clip_size)
{
    if (N <= clip_size * 2)
        return N / 2;
    return clip_size + (qrand() % (N - clip_size * 2));
}

QVector<double> sample(const QVector<double>& times, long num)
{
    QVector<double> random_values(times.count());
    for (long i = 0; i < times.count(); i++) {
        random_values[i] = sin(i * 12 + i * i);
    }
    QList<long> inds = get_sort_indices(random_values);
    QVector<double> ret;
    for (long i = 0; (i < num) && (i < inds.count()); i++) {
        ret << times[inds[i]];
    }
    return ret;
}

double compute_noise_overlap(const DiskReadMda32& X, const QVector<double>& times, ms_metrics_opts opts)
{
    int num_to_use = qMin(opts.max_num_to_use, times.count());
    QVector<double> times_subset = sample(times, num_to_use);

    QVector<double> all_times = times_subset;
    QVector<int> all_labels; //0 and 1

    for (long i = 0; i < times_subset.count(); i++) {
        all_labels << 1;
    }
    //equal amount of random clips
    for (long i = 0; i < times_subset.count(); i++) {
        all_times << random_time(X.N2(), opts.clip_size);
        all_labels << 0;
    }

    Mda32 all_clips = extract_clips(X, all_times, opts.clip_size);

    Mda32 all_clips_reshaped(all_clips.N1() * all_clips.N2(), all_clips.N3());
    long NNN = all_clips.totalSize();
    for (long iii = 0; iii < NNN; iii++) {
        all_clips_reshaped.set(all_clips.get(iii), iii);
    }

    bool subtract_mean = false;
    Mda32 FF;
    Mda32 CC, sigma;
    pca(CC, FF, sigma, all_clips_reshaped, opts.num_features, subtract_mean);

    KdTree tree;
    tree.create(FF);
    double num_correct = 0;
    double num_total = 0;
    for (long i = 0; i < FF.N2(); i++) {
        QVector<float> p;
        for (int j = 0; j < FF.N1(); j++) {
            p << FF.value(j, i);
        }
        QList<long> indices = tree.findApproxKNearestNeighbors(FF, p, opts.K_nearest, opts.exhaustive_search_num);
        for (int a = 0; a < indices.count(); a++) {
            if (indices[a] != i) {
                if (all_labels[indices[a]] == all_labels[i])
                    num_correct++;
                num_total++;
            }
        }
    }
    if (!num_total)
        return 0;
    return 1 - (num_correct * 1.0 / num_total);
}

double compute_overlap(const DiskReadMda32& X, const QVector<double>& times1, const QVector<double>& times2, ms_metrics_opts opts)
{
    int num_to_use = qMin(qMin(opts.max_num_to_use, times1.count()), times2.count());
    if (num_to_use < opts.min_num_to_use)
        return 0;
    QVector<double> times1_subset = sample(times1, num_to_use);
    QVector<double> times2_subset = sample(times2, num_to_use);

    QVector<double> all_times;
    QVector<int> all_labels; //1 and 2

    for (long i = 0; i < times1_subset.count(); i++) {
        all_times << times1_subset[i];
        all_labels << 1;
    }
    for (long i = 0; i < times2_subset.count(); i++) {
        all_times << times2_subset[i];
        all_labels << 2;
    }

    Mda32 all_clips = extract_clips(X, all_times, opts.clip_size);

    Mda32 all_clips_reshaped(all_clips.N1() * all_clips.N2(), all_clips.N3());
    long NNN = all_clips.totalSize();
    for (long iii = 0; iii < NNN; iii++) {
        all_clips_reshaped.set(all_clips.get(iii), iii);
    }

    bool subtract_mean = false;
    Mda32 FF;
    Mda32 CC, sigma;
    pca(CC, FF, sigma, all_clips_reshaped, opts.num_features, subtract_mean);

    KdTree tree;
    tree.create(FF);
    double num_correct = 0;
    double num_total = 0;
    for (long i = 0; i < all_times.count(); i++) {
        QVector<float> p;
        for (int j = 0; j < FF.N1(); j++) {
            p << FF.value(j, i);
        }
        QList<long> indices = tree.findApproxKNearestNeighbors(FF, p, opts.K_nearest, opts.exhaustive_search_num);
        for (int a = 0; a < indices.count(); a++) {
            if (indices[a] != i) {
                if (all_labels[indices[a]] == all_labels[i])
                    num_correct++;
                num_total++;
            }
        }
    }
    if (!num_total)
        return 0;
    return 1 - (num_correct * 1.0 / num_total);
}

QVector<long> find_label_inds(const QVector<int>& labels, int k)
{
    QVector<long> ret;
    for (int i = 0; i < labels.count(); i++) {
        if (labels[i] == k)
            ret << i;
    }
    return ret;
}

bool peaks_are_within_range_to_consider(double p1, double p2, ms_metrics_opts opts)
{
    if ((!p1) || (!p2))
        return false;
    double ratio = p1 / p2;
    if (ratio > 1)
        ratio = 1 / ratio;
    if (ratio < 0)
        return false;
    return (ratio > opts.min_peak_ratio_to_consider);
}

bool peaks_are_within_range_to_consider(double p11, double p12, double p21, double p22, ms_metrics_opts opts)
{
    return (
        (peaks_are_within_range_to_consider(p11, p12, opts)) && (peaks_are_within_range_to_consider(p21, p22, opts)) && (peaks_are_within_range_to_consider(p11, p21, opts)) && (peaks_are_within_range_to_consider(p12, p22, opts)));
}

QSet<QString> get_pairs_to_consider(const DiskReadMda32& X, const DiskReadMda& F, ms_metrics_opts opts)
{
    QVector<int> peakchans;
    QVector<double> times;
    QVector<int> labels;
    for (long i = 0; i < F.N2(); i++) {
        peakchans << F.value(0, i);
        times << F.value(1, i);
        labels << F.value(2, i);
    }

    //compute the average waveforms (aka templates)
    Mda32 templates = compute_templates_0(X, times, labels, opts.clip_size);
    int M = templates.N1();
    int T = templates.N2();
    int K = templates.N3();

    Mda32 channel_peaks(M, K);
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
    QSet<QString> ret;
    for (int k1 = 1; k1 <= K; k1++) {
        QVector<long> inds1 = find_label_inds(labels, k1);
        if (!inds1.isEmpty()) {

            for (int k2 = 1; k2 <= K; k2++) {
                ret.insert(QString("%1-%2").arg(k1).arg(k2));
                /*
                QVector<long> inds2 = find_label_inds(labels, k2);
                if (!inds2.isEmpty()) {
                    int peakchan1 = peakchans[inds1[0]]; //the peak channel should be the same for all events with this labels, so we just need to look at the first one
                    int peakchan2 = peakchans[inds2[0]];
                    if (peakchan1 != peakchan2) { //only attempt to merge if the peak channels are different -- that's why it's called "merge_across_channels"
                        double val11 = channel_peaks.value(peakchan1 - 1, k1-1);
                        double val12 = channel_peaks.value(peakchan2 - 1, k1-1);
                        double val21 = channel_peaks.value(peakchan1 - 1, k2-1);
                        double val22 = channel_peaks.value(peakchan2 - 1, k2-1);
                        if (peaks_are_within_range_to_consider(val11, val12, val21, val22, opts)) {
                            ret.insert(QString("%1-%2").arg(k1).arg(k2));
                        }
                    }
                }
                */
            }
        }
    }
    return ret;
}
}
