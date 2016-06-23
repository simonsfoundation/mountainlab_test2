#include "mv_discrimhist.h"

#include "diskreadmda.h"
#include "extract_clips.h"
#include "msmisc.h"

struct discrimhist_data {
    int k1, k2;
    QVector<double> data1;
    QVector<double> data2;
};

/// TODO parallelize mv_distrimhist

void get_discrimhist_data(QVector<double>& ret1, QVector<double>& ret2, const DiskReadMda& timeseries, const DiskReadMda& firings, int k1, int k2, int clip_size);
bool mv_discrimhist(QString timeseries_path, QString firings_path, QString output_path, mv_discrimhist_opts opts)
{
    DiskReadMda timeseries(timeseries_path);
    DiskReadMda firings(firings_path);

    QList<discrimhist_data> datas;
    for (int i1 = 0; i1 < opts.clusters.count(); i1++) {
        for (int i2 = i1 + 1; i2 < opts.clusters.count(); i2++) {
            int k1 = opts.clusters[i1];
            int k2 = opts.clusters[i2];
            discrimhist_data DD;
            DD.k1 = k1;
            DD.k2 = k2;
            get_discrimhist_data(DD.data1, DD.data2, timeseries, firings, k1, k2, opts.clip_size);
            datas << DD;
        }
    }

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

double compute_dot_product(long N, double* v1, double* v2)
{
    double ip = 0;
    for (int m = 0; m < N; m++)
        ip += v1[m] * v2[m];
    return ip;
}

void get_discrimhist_data(QVector<double>& ret1, QVector<double>& ret2, const DiskReadMda& timeseries, const DiskReadMda& firings, int k1, int k2, int clip_size)
{
    QList<double> times1, times2;
    for (long i = 0; i < firings.N2(); i++) {
        int label = (int)firings.value(2, i);
        if (label == k1) {
            times1 << firings.value(1, i);
        }
        if (label == k2) {
            times2 << firings.value(1, i);
        }
    }
    Mda clips1 = extract_clips(timeseries, times1, clip_size);
    Mda clips2 = extract_clips(timeseries, times2, clip_size);

    Mda centroid1 = compute_mean_clip(clips1);
    Mda centroid2 = compute_mean_clip(clips2);

    Mda diff(centroid1.N1(), centroid1.N2());
    for (long i2 = 0; i2 < centroid1.N2(); i2++) {
        for (long i1 = 0; i1 < centroid1.N1(); i1++) {
            diff.setValue(centroid2.value(i1, i2) - centroid1.value(i1, i2), i1, i2);
        }
    }

    double* ptr_clips1 = clips1.dataPtr();
    double* ptr_clips2 = clips2.dataPtr();

    long N = centroid1.N1() * centroid1.N2();
    double* ptr_diff = diff.dataPtr();
    double norm0 = compute_norm(N, ptr_diff);
    if (!norm0)
        norm0 = 1;

    ret1.clear();
    for (long i = 0; i < clips1.N3(); i++) {
        ret1 << compute_dot_product(N, ptr_diff, &ptr_clips1[N * i]) / (norm0 * norm0);
    }
    ret2.clear();
    for (long i = 0; i < clips2.N3(); i++) {
        ret2 << compute_dot_product(N, ptr_diff, &ptr_clips2[N * i]) / (norm0 * norm0);
    }
}
