#include "compute_amplitudes.h"

#include "compute_templates_0.h"
#include "extract_clips.h"
#include "mlcommon.h"

#include <QFile>

QVector<long> find_label_inds_00(const QVector<int>& labels, int k);
double compute_max_amp_of_template(long* max_amp_index, const Mda32& template0);

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
    int A = qMax(firings.N1(), 4L);
    Mda firings_out(A, firings.N2());
    for (long i = 0; i < firings.N2(); i++) {
        for (int a = 0; a < A; a++) {
            firings_out.setValue(firings.value(a, i), a, i);
        }
        firings_out.setValue(0, 3, i);
    }
    for (int k = 1; k <= K; k++) {
        QVector<long> inds_k = find_label_inds_00(labels, k);
        QVector<double> times_k(inds_k.count());
        for (long i = 0; i < inds_k.count(); i++) {
            times_k[i] = times[inds_k[i]];
        }
        Mda32 template0;
        templates.getChunk(template0, 0, 0, k - 1, templates.N1(), templates.N2(), 1);
        long max_amp_index;
        double max_amp = compute_max_amp_of_template(&max_amp_index, template0);
        //double template_norm=MLCompute::norm(template0.totalSize(),template0.constDataPtr());
        if (max_amp) {
            Mda32 clips_k = extract_clips(X, times_k, opts.clip_size);
            for (long i = 0; i < inds_k.count(); i++) {
                Mda32 clip0;
                clips_k.getChunk(clip0, 0, 0, i, clips_k.N1(), clips_k.N2(), 1);
                /*
                //this was the tricky way to do it, but the simpler way is much better
                double tmp = MLCompute::dotProduct(clip0.N1() * clip0.N2(), clip0.dataPtr(), template0.dataPtr());
                double amp0=tmp/(template_norm*template_norm)*max_amp;
                */
                /// TODO: This involves a lot of extra unnecessary computations
                double amp0 = clip0.value(max_amp_index);
                firings_out.setValue(amp0, 3, inds_k[i]);
            }
        }
    }

    return firings_out.write64(firings_out_path);
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

double compute_max_amp_of_template(long* max_amp_index, const Mda32& template0)
{
    long N = template0.totalSize();
    const float* ptr = template0.constDataPtr();
    double ret = 0;
    *max_amp_index = 0;
    for (long i = 0; i < N; i++) {
        if (qAbs(ptr[i]) > qAbs(ret)) {
            ret = ptr[i];
            *max_amp_index = i;
        }
    }
    return ret;
}
