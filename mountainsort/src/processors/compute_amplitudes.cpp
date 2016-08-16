#include "compute_amplitudes.h"

#include "compute_templates_0.h"
#include "extract_clips.h"
#include "mlcommon.h"

QVector<long> find_label_inds_00(const QVector<int>& labels, int k);

bool compute_amplitudes(QString timeseries_path, QString firings_path, QString firings_out_path, compute_amplitudes_opts opts)
{
    DiskReadMda32 X(timeseries_path);
    DiskReadMda firings(firings_path);
    QVector<double> times;
    QVector<int> labels;
    for (long i = 0; i < firings.N2(); i++) {
        times << firings.value(1, i);
        labels << firings.value(2, i);
    }
    Mda32 templates = compute_templates_0(X, times, labels, opts.clip_size);
    int K = MLCompute::max(labels);
    Mda firings_out(firings.N1(), firings.N2());
    for (long i = 0; i < firings.N2(); i++) {
        for (int a = 0; a < firings.N1(); a++) {
            firings_out.setValue(firings.value(a, i), a, i);
        }
    }
    for (int k = 1; k <= K; k++) {
        QVector<long> inds_k = find_label_inds_00(labels, k);
        QVector<double> times_k(inds_k.count());
        for (long i = 0; i < inds_k.count(); i++) {
            times_k[i] = times[inds_k[i]];
        }
        Mda32 template0;
        templates.getChunk(template0, 0, 0, k - 1, template0.N1(), template0.N2(), 1);
        Mda32 clips_k = extract_clips(X, times_k, opts.clip_size);
        for (long i = 0; i < inds_k.count(); i++) {
            Mda32 clip0;
            clips_k.getChunk(template0, 0, 0, i, clips_k.N1(), clips_k.N2(), 1);
            double tmp = MLCompute::dotProduct(clip0.N1() * clip0.N2(), clip0.dataPtr(), template0.dataPtr());
            /// FINISH!
        }
    }
    return true;
}

QVector<long> find_label_inds_00(const QVector<int>& labels, int k)
{
    QVector<long> ret;
    for (int i = 0; i < labels.count(); i++) {
        if (labels[i] == k)
            ret << i;
    }
    return ret;
}
