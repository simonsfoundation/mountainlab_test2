/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 9/13/2016
*******************************************************/

#include "noise_nearest.h"

#include <diskreadmda.h>
#include <diskreadmda32.h>
#include "mlcommon.h"
#include "extract_clips.h"
#include "synthesize1.h" //for randn
#include "msmisc.h"
#include "compute_templates_0.h"
#include "jsvm.h"

namespace NoiseNearest {

class DecisionTree {
public:
    virtual ~DecisionTree()
    {
        if (m_child1)
            delete m_child1;
        if (m_child2)
            delete m_child2;
    }
    void learn(const Mda32& X, QVector<int> labels)
    {
        if (has_at_most_one_value(labels)) {
            m_label = labels.value(0);
            return;
        }
        else {
            //m_projection_direction=random_projection_direction(X.N1());
            m_projection_direction = get_projection_direction(X);
            QVector<double> vals;
            const float* ptr = X.constDataPtr();
            for (long i = 0; i < X.N2(); i++) {
                vals << MLCompute::dotProduct(X.N1(), m_projection_direction.data(), &ptr[X.N1() * i]);
            }
            QVector<double> vals_sorted = vals;
            qSort(vals_sorted);
            m_cutoff = vals_sorted.value(vals_sorted.count() / 2);
            QList<long> inds1, inds2;
            for (long i = 0; i < X.N2(); i++) {
                if (vals.value(i) < m_cutoff)
                    inds1 << i;
                else
                    inds2 << i;
            }
            if ((inds1.isEmpty()) || (inds2.isEmpty())) {
                //must have been an exact equality
                qWarning() << "There must have been an exact equality in DecisionTree" << inds1.count() << inds2.count();
                m_label = labels.value(0);
                return;
            }
            {
                Mda32 Xchild(X.N1(), inds1.count());
                QVector<int> labels_child;
                for (long i = 0; i < inds1.count(); i++) {
                    for (int j = 0; j < X.N1(); j++) {
                        Xchild.setValue(X.value(j, inds1[i]), j, i);
                    }
                    labels_child << labels.value(inds1[i]);
                }
                m_child1 = new DecisionTree;
                m_child1->learn(Xchild, labels_child);
            }
            {
                Mda32 Xchild(X.N1(), inds2.count());
                QVector<int> labels_child;
                for (long i = 0; i < inds2.count(); i++) {
                    for (int j = 0; j < X.N1(); j++) {
                        Xchild.setValue(X.value(j, inds2[i]), j, i);
                    }
                    labels_child << labels.value(inds2[i]);
                }
                m_child2 = new DecisionTree;
                m_child2->learn(Xchild, labels_child);
            }
        }
    }
    QVector<int> classify(const Mda32& X)
    {
        QVector<int> ret;
        for (long i = 0; i < X.N2(); i++) {
            QVector<float> tmp;
            for (int j = 0; j < X.N1(); j++)
                tmp << X.value(j, i);
            ret << classify(tmp);
        }
        return ret;
    }

    int classify(const QVector<float>& X)
    {
        if (!m_child1)
            return m_label;
        double val = MLCompute::dotProduct(m_projection_direction, X);
        if (val < m_cutoff) {
            return m_child1->classify(X);
        }
        else {
            return m_child2->classify(X);
        }
    }

private:
    QVector<float> m_projection_direction;
    double m_cutoff = 0;

    int m_label = 0;
    DecisionTree* m_child1 = 0;
    DecisionTree* m_child2 = 0;

    bool has_at_most_one_value(const QVector<int>& labels)
    {
        QSet<int> set;
        for (long i = 0; i < labels.count(); i++) {
            set.insert(labels[i]);
            if (set.count() > 1)
                return false;
        }
        return (set.count() <= 1);
    }
    QVector<float> random_projection_direction(int M)
    {
        //Not really a random direction. :(
        QVector<float> ret;
        for (int i = 0; i < M; i++) {
            double val = (qrand() % RAND_MAX) * 1.0 / RAND_MAX;
            ret << val * 2 - 1;
        }
        double norm0 = MLCompute::norm(ret);
        if (norm0) {
            for (int i = 0; i < M; i++) {
                ret[i] /= norm0;
            }
        }
        return ret;
    }
    QVector<float> get_projection_direction(const Mda32& X)
    {
        Mda32 CC, FF, sig;
        pca(CC, FF, sig, X, 1, false);
        QVector<float> ret;
        for (int i = 0; i < X.N1(); i++)
            ret << CC.value(i);
        return ret;
    }
};

Mda32 add_self_noise_to_clips(const DiskReadMda32& timeseries, const Mda32& clips, double noise_factor);
Mda32 concat_clips(const Mda32& clips1, const Mda32& clips2);

bool noise_nearest(QString timeseries, QString firings, QString confusion_matrix, noise_nearest_opts opts)
{
    Mda CM = compute_isolation_matrix(timeseries, firings, opts);

    printf("Writing output...\n");
    return CM.write64(confusion_matrix);
}

Mda32 concat_clips(const Mda32& clips1, const Mda32& clips2)
{
    Mda32 ret(clips1.N1(), clips1.N2(), clips1.N3() + clips2.N3());
    for (long i = 0; i < clips1.N3(); i++) {
        Mda32 tmp;
        clips1.getChunk(tmp, 0, 0, i, clips1.N1(), clips1.N2(), 1);
        ret.setChunk(tmp, 0, 0, i);
    }
    for (long i = 0; i < clips2.N3(); i++) {
        Mda32 tmp;
        clips2.getChunk(tmp, 0, 0, i, clips2.N1(), clips2.N2(), 1);
        ret.setChunk(tmp, 0, 0, i + clips1.N3());
    }
    return ret;
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

Mda compute_isolation_matrix(QString timeseries, QString firings, noise_nearest_opts opts)
{
    DiskReadMda32 X(timeseries);
    DiskReadMda F(firings);
    int num_features = 0;

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

    printf("Adding random events...\n");
    long num_evts = times.count();
    for (long i = 0; i < num_evts; i++) {
        times << opts.clip_size + (qrand() % X.N2() - opts.clip_size * 2);
        labels << 0;
    }

    printf("Extracting clips and adding noise\n");
    Mda32 clips = extract_clips(X, times, opts.clip_size);
    Mda32 clips_plus_noise = add_self_noise_to_clips(X, clips, opts.add_noise_level);
    Mda32 all_clips = concat_clips(clips, clips_plus_noise);

    printf("Computing PCA features...\n");
    long M_all = all_clips.N1();
    long T_all = all_clips.N2();
    long L_all = all_clips.N3();
    Mda32 all_clips_reshaped(M_all * T_all, L_all);
    long NNN = M_all * T_all * L_all;
    for (long iii = 0; iii < NNN; iii++) {
        all_clips_reshaped.set(all_clips.get(iii), iii);
    }
    bool subtract_mean = false;
    Mda32 FF;
    if (num_features) {
        Mda32 CC, sigma;
        pca(CC, FF, sigma, all_clips_reshaped, num_features, subtract_mean);
    }
    else {
        FF = all_clips_reshaped;
        num_features = all_clips_reshaped.N1();
    }

    printf("Assembling features...\n");
    Mda32 FF_clips(num_features, clips.N3());
    for (long i = 0; i < clips.N3(); i++) {
        for (int j = 0; j < num_features; j++) {
            FF_clips.setValue(FF.value(j, i), j, i);
        }
    }
    Mda32 FF_clips_plus_noise(num_features, clips.N3());
    for (long i = 0; i < clips_plus_noise.N3(); i++) {
        for (int j = 0; j < num_features; j++) {
            FF_clips_plus_noise.setValue(FF.value(j, i + clips.N3()), j, i);
        }
    }

    printf("Learning decision tree...\n");
    DecisionTree DT;
    DT.learn(FF_clips, labels);

    printf("Classifying noisy events...\n");
    QVector<int> labels_after_noise = DT.classify(FF_clips_plus_noise);

    printf("Calculating label map...\n");
    int KK = opts.cluster_numbers.count();
    QList<int> label_map;
    for (int a = 0; a <= K; a++) {
        label_map << 0;
    }
    for (int i = 0; i < opts.cluster_numbers.count(); i++) {
        if (opts.cluster_numbers[i] <= K) {
            label_map[opts.cluster_numbers[i]] = i;
        }
    }
    label_map[0] = opts.cluster_numbers.count(); //unclassified

    printf("Defining confusion matrix...\n");
    Mda CM(KK + 1, KK + 1);
    for (long i = 0; i < labels.count(); i++) {
        int k1 = label_map[labels[i]];
        int k2 = label_map[labels_after_noise[i]];
        if ((0 <= k1) && (k1 < KK + 1) && (0 <= k2) && (k2 < KK + 1)) {
            CM.setValue(CM.value(k1, k2) + 1, k1, k2);
        }
    }

    return CM;
}
}
