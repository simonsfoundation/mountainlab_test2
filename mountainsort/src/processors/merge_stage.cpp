/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/22/2016
*******************************************************/

#include "merge_stage.h"
#include "diskreadmda.h"
#include "mlcommon.h"
#include "mlcommon.h"
#include "compute_templates_0.h"
#include "fit_stage.h"
#include "isosplit2.h"
#include <math.h>
#include "extract_clips.h"
#include "msmisc.h"
#include "isocut.h"

bool check_if_merge_candidate(double& best_dt0, Mda& template1, Mda& template2, double peakchan1, double peakchan2, const merge_stage_opts& opts);
void make_reflexive_and_transitive_2(Mda& S, Mda& best_dt);
Mda remove_redundant_events_2(Mda& firings, int maxdt);
QVector<int> remove_unused_labels_2(const QVector<int>& labels);
bool test_for_merge(const QVector<double> &X1, const QVector<double> &X2);
bool test_for_merge(const Mda &clips1, const Mda &clips2);

bool merge_stage(const QString& timeseries_path, const QString& firings_path, const QString& firings_out_path, const merge_stage_opts& opts)
{
    DiskReadMda X(timeseries_path);
    Mda firings;
    firings.read(firings_path);

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

    Mda templates = compute_templates_0(X, times, labels, opts.clip_size);

    //assemble the merge_matrix (binary) and best_dt
    printf("Assembling the merge matrix...\n");
    Mda merge_matrix(K, K);
    Mda best_dt(K, K);
    for (int k1 = 1; k1 <= K; k1++) {
        for (int k2 = k1+1; k2 <= K; k2++) {
            QList<long> inds1 = find_inds(labels, k1);
            QList<long> inds2 = find_inds(labels, k2);
            if ((!inds1.isEmpty()) && (!inds2.isEmpty())) {
                Mda template1, template2;
                templates.getChunk(template1, 0, 0, k1 - 1, M, T, 1);
                templates.getChunk(template2, 0, 0, k2 - 1, M, T, 1);
                int peakchan1 = peakchans[inds1[0]];
                int peakchan2 = peakchans[inds2[0]];
                double best_dt0;
                if (check_if_merge_candidate(best_dt0,template1,template2,peakchan1,peakchan2,opts)) {
                    printf("Candidate: (%d,%d) dt=%g\n",k1,k2,best_dt0);
                    QVector<double> times1, times2;
                    for (long a = 0; a < inds1.count(); a++)
                        times1 << times[inds1[a]];
                    for (long a = 0; a < inds2.count(); a++)
                        times2 << times[inds2[a]] - best_dt0; //do the time correction (very important)
                    Mda clips1=extract_clips(X,times1,opts.clip_size);
                    Mda clips2=extract_clips(X,times2,opts.clip_size);
                    if (test_for_merge(clips1,clips2)) {
                        merge_matrix.setValue(1, k1 - 1, k2 - 1);
                        best_dt.setValue(best_dt0, k1 - 1, k2 - 1);
                    }
                }
            }
        }
    }

    //make the matrix reflexive and transitive
    make_reflexive_and_transitive_2(merge_matrix, best_dt);

    //now we merge based on the above scores
    QVector<int> new_labels = labels;
    QVector<double> new_times = times;
    for (int k1 = 1; k1 <= K; k1++) {
        for (int k2 = k1+1; k2 <= K; k2++) {
            if (merge_matrix.value(k1 - 1, k2 - 1)) {
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
    new_labels = remove_unused_labels_2(new_labels);
    int K_new = MLCompute::max<int>(new_labels);
    printf("Merged into %d of %d clusters\n", K_new, K);

    //set the output
    Mda firings_out = firings;
    for (long i = 0; i < firings_out.N2(); i++) {
        firings_out.setValue(new_times[i], 1, i);
        firings_out.setValue(new_labels[i], 2, i);
    }

    //Now we may have a bunch of redundant events! So let's remove them!
    long maxdt = 5;
    firings_out = remove_redundant_events_2(firings_out, maxdt);

    printf("Using %ld of %ld events\n", firings_out.N2(), firings.N2());

    firings_out.write64(firings_out_path);

    return true;
}

void make_reflexive_and_transitive_2(Mda& S, Mda& best_dt)
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

double max_absolute_value_on_channel_2(Mda& template1, int channel)
{
    QVector<double> vals;
    for (int i = 0; i < template1.N2(); i++) {
        vals << qAbs(template1.value(channel - 1, i));
    }
    return MLCompute::max(vals);
}

double compute_noncentered_correlation_2(long N, double* X1, double* X2)
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

Mda time_shift_template_2(Mda& template0, int dt)
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

double compute_sliding_noncentered_correlation_2(Mda& template1, Mda& template2)
{
    int M = template1.N1();
    int T = template1.N2();
    double best = 0;
    for (int dt = -T / 2; dt <= T / 2; dt++) {
        Mda template1b = time_shift_template_2(template1, dt);
        double val = compute_noncentered_correlation_2(M * T, template1b.dataPtr(), template2.dataPtr());
        if (val > best)
            best = val;
    }
    return best;
}

bool check_if_merge_candidate(double& best_dt0, Mda& template1, Mda& template2, double peakchan1, double peakchan2, const merge_stage_opts& opts)
{
    //values to return if no merge
    best_dt0 = 0;

    //min_peak_ratio criterion
    double template1_self_peak = max_absolute_value_on_channel_2(template1, peakchan1);
    double template1_other_peak = max_absolute_value_on_channel_2(template1, peakchan2);
    if (template1_other_peak < opts.min_peak_ratio * template1_self_peak) {
        return false;
    }

    //min_template_corr_coef criterion
    //double r12 = compute_noncentered_correlation_2(M * T, template1.dataPtr(), template2.dataPtr());
    double r12 = compute_sliding_noncentered_correlation_2(template1, template2);
    if (r12 < opts.min_template_corr_coef) {
        return false;
    }

    return true;
}

Mda remove_redundant_events_2(Mda& firings, int maxdt)
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

QVector<int> remove_unused_labels_2(const QVector<int>& labels)
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

QVector<double> everything_below(const QVector<double> &X,double val) {
    QVector<double> ret;
    for (long i=0; i<X.count(); i++) {
        if (X[i]<val)
            ret << X[i];
    }
    return ret;
}

QVector<double> everything_above(const QVector<double> &X,double val) {
    QVector<double> ret;
    for (long i=0; i<X.count(); i++) {
        if (X[i]>=val)
            ret << X[i];
    }
    return ret;
}

bool test_for_merge(const QVector<double> &X1, const QVector<double> &X2) {

    double mean1=MLCompute::mean(X1);
    double mean2=MLCompute::mean(X2);
    if (mean2<mean1) {
        //let's guarantee that mean1<mean2
        return test_for_merge(X2,X1);
    }

    QVector<double> X;
    X.append(X1);
    X.append(X2);
    qSort(X);
    double isocut_threshold=1.5;
    int minsize=5;
    double cutpoint;
    if (!isocut(X.count(),&cutpoint,X.data(),isocut_threshold,minsize)) {
        //No split, therefore we will merge
        printf("Test for merge: *********************** MERGE **********************\n");
        return true;
    }

    //Otherwise, we need to test the individual distributions to see if they split on their own...
    double cutpoint1;
    if (isocut(X1.count(),&cutpoint1,X1.data(),isocut_threshold,minsize)) {
        //the first cluster is not unimodal! So let's cut out the lower part and repeat
        QVector<double> X1_above=everything_above(X1,cutpoint1);
        QVector<double> X2_above=everything_above(X2,cutpoint1);
        printf("Test for merge: TRY AGAIN +\n");
        return test_for_merge(X1_above,X2_above);
    }
    double cutpoint2;
    if (isocut(X2.count(),&cutpoint2,X2.data(),isocut_threshold,minsize)) {
        //the second cluster is not unimodal! So let's cut out the upper part and repeat
        QVector<double> X1_above=everything_above(X1,cutpoint2);
        QVector<double> X2_above=everything_above(X2,cutpoint2);
        printf("Test for merge: TRY AGAIN -\n");
        return test_for_merge(X1_above,X2_above);
    }
    //It seems that the two clusters were individually unimodal, so we stand by the split
    return false;
}

bool test_for_merge(const Mda &clips1, const Mda &clips2) {
    int M=clips1.N1();
    int T=clips1.N2();
    long L1=clips1.N3();
    long L2=clips2.N3();

    Mda W1=compute_mean_clip(clips1);
    Mda W2=compute_mean_clip(clips2);
    Mda Wdiff(M,T);
    for (long ii=0; ii<M*T; ii++) {
        Wdiff.setValue(W2.value(ii)-W1.value(ii),ii);
    }

    const double *ptr1=clips1.constDataPtr();
    const double *ptr2=clips2.constDataPtr();
    const double *Wdiff_ptr=Wdiff.dataPtr();

    QVector<double> inner_products_1(L1);
    for (long i=0; i<L1; i++) {
        inner_products_1[i]=MLCompute::dotProduct(M*T,Wdiff_ptr,&ptr1[M*T*i]);
    }
    QVector<double> inner_products_2(L2);
    for (long i=0; i<L2; i++) {
        inner_products_2[i]=MLCompute::dotProduct(M*T,Wdiff_ptr,&ptr2[M*T*i]);
    }

    return test_for_merge(inner_products_1,inner_products_2);
}
