/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 9/13/2016
*******************************************************/

#include "ms_metrics.h"

#include <diskreadmda.h>
#include <diskreadmda32.h>
#include "mlcommon.h"
#include "extract_clips.h"
#include "synthesize1.h" //for randn
#include "msmisc.h"
#include "compute_templates_0.h"
#include "jsvm.h"
#include "noise_nearest.h"

namespace MSMetrics {

struct Metric {
    QList<double> values;
};

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

    noise_nearest_opts nn_opts;
    nn_opts.cluster_numbers = opts.cluster_numbers;
    nn_opts.clip_size = opts.clip_size;
    nn_opts.add_noise_level = opts.add_noise_level;
    Mda isolation_matrix = NoiseNearest::compute_isolation_matrix(timeseries, firings, nn_opts);

    QMap<QString, Metric> cluster_metrics;
    for (int i = 0; i < opts.cluster_numbers.count(); i++) {
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

    //////////////////////////////////////////////////////////////
    QMap<QString, Metric> cluster_pair_metrics;
    for (int i1 = 0; i1 < opts.cluster_numbers.count(); i1++) {
        for (int i2 = 0; i2 < opts.cluster_numbers.count(); i2++) {
            int k1 = opts.cluster_numbers[i1];
            //int k2 = opts.cluster_numbers[i2];
            QVector<double> times_k1;
            for (long i = 0; i < times.count(); i++) {
                if (labels[i] == k1) {
                    times_k1 << times[i];
                }
            }
            double numer = isolation_matrix.value(i1, i2);
            double denom = times_k1.count();
            if (!denom)
                denom = 1;
            double val = numer / denom;
            if (val < 0.01)
                val = 0; //to save space in the .mv2 file, don't include anything less than 1%
            cluster_pair_metrics["isolation_matrix"].values << val;
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
}
