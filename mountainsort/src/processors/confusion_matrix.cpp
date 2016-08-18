#include "confusion_matrix.h"
#include "diskreadmda.h"
#include <QSet>
#include <QMap>
#include "get_sort_indices.h"

QVector<int> indexlist(const QVector<int>& T2, int t1, int offset, int& ptr2);
Mda confusion_matrix_2(const char* firings1_path, const char* firings2_path, int max_matching_offset, QMap<int, int>& map12);

bool confusion_matrix(const char* firings1_path, const char* firings2_path, const char* output_path, int max_matching_offset)
{
    //first we get the confusion matrix with an empty map12
    QMap<int, int> empty_map;
    Mda CM = confusion_matrix_2(firings1_path, firings2_path, max_matching_offset, empty_map);

    //next we estimate the map12 based on CM
    QMap<int, int> map12;
    int K1 = CM.N1() - 1;
    int K2 = CM.N2() - 1;
    for (int k1 = 1; k1 <= K1; k1++) {
        int best_k2 = 0;
        int best_count = 0;
        for (int k2 = 1; k2 <= K2; k2++) {
            int count = (int)CM.value(k1 - 1, k2 - 1);
            if (count > best_count) {
                best_k2 = k2;
                best_count = count;
            }
        }
        map12[k1] = best_k2;
    }

    //finally we get the confusion matrix again using this map12
    Mda output = confusion_matrix_2(firings1_path, firings2_path, max_matching_offset, map12);

    //finally we write it
    output.write32(output_path);

    return true;
}

void sort_times_labels(QVector<double> &times, QVector<int> &labels) {
    QVector<double> times2;
    QVector<int> labels2;
    QList<long> inds=get_sort_indices(times);
    for (long i=0; i<inds.count(); i++) {
        times2 << times[inds[i]];
        labels2 << labels[inds[i]];
    }
    times=times2;
    labels=labels2;
}

Mda confusion_matrix_2(const char* firings1_path, const char* firings2_path, int max_matching_offset, QMap<int, int>& map12)
{
    DiskReadMda C1;
    C1.setPath(firings1_path);
    DiskReadMda C2;
    C2.setPath(firings2_path);

    QVector<double> T1d,T2d;
    QVector<int> L1,L2;

    for (int ii = 0; ii < C1.N2(); ii++) {
        T1d << C1.value(1, ii);
        L1 << C1.value(2, ii);
    }
    for (int ii = 0; ii < C2.N2(); ii++) {
        T2d << C2.value(1, ii);
        L2 << C2.value(2, ii);
    }

    sort_times_labels(T1d,L1);
    sort_times_labels(T2d,L2);

    QVector<int> T1,T2;
    for (long i=0; i<T1d.count(); i++) {
        T1 << (int)T1d[i];
    }
    for (long i=0; i<T2d.count(); i++) {
        T2 << (int)T2d[i];
    }

    int K1 = 1;
    for (int ii = 0; ii < L1.count(); ii++) {
        if (L1[ii] > K1)
            K1 = L1[ii];
    }
    int K2 = 1;
    for (int ii = 0; ii < L2.count(); ii++) {
        if (L2[ii] > K2)
            K2 = L2[ii];
    }

    int CM[(K1 + 1) * (K2 + 1)];
    for (int ii = 0; ii < (K1 + 1) * (K2 + 1); ii++)
        CM[ii] = 0;

    int pass1 = 1;
    int pass2 = 2;
    if (map12.isEmpty())
        pass1 = 2; //if the map is empty, only do pass 2

    for (int pass = pass1; pass <= pass2; pass++) {
        //on the first pass we are giving priority to matches that agree with map12
        for (int offset = 0; offset <= max_matching_offset; offset++) {
            QSet<int> inds1_to_remove;
            inds1_to_remove.clear(); //clear almost definitely is not needed here, but I was a bit nervous, better safe than sorry
            QSet<int> inds2_to_remove;
            inds2_to_remove.clear();
            int ptr2 = 0; //moving index to sorted T2 list
            for (int ii1 = 0; ii1 < T1.count(); ii1++) {
                //find indices of C2 which should be paired with the event at ii1
                QVector<int> ii2_list = indexlist(T2, T1[ii1], offset, ptr2);
                //only use those that haven't been removed!
                QVector<int> ii2_list_b;
                for (int j = 0; j < ii2_list.count(); j++) {
                    if (!inds2_to_remove.contains(ii2_list[j])) {
                        if (pass == 1) {
                            //only use those that agree with the mapping
                            if (map12[L1[ii1]] == L2[ii2_list[j]]) {
                                ii2_list_b << ii2_list[j];
                            }
                        }
                        else {
                            ii2_list_b << ii2_list[j];
                        }
                    }
                }
                if (ii2_list_b.count() > 0) {
                    //let's only use the first
                    int ii2 = ii2_list_b[0];
                    int l1 = L1[ii1];
                    int l2 = L2[ii2];
                    CM[(l1 - 1) + (K1 + 1) * (l2 - 1)]++; //increment the entry in the confusion matrix
                    inds1_to_remove.insert(ii1); //we've handled the event, so let's remove it!
                    inds2_to_remove.insert(ii2); //we've handled the event, so let's remove it!
                }
            }
            //now remove the events that were marked above
            QVector<int> new_T1, new_L1;
            for (int i = 0; i < T1.count(); i++) {
                if (!inds1_to_remove.contains(i)) {
                    new_T1 << T1[i];
                    new_L1 << L1[i];
                }
            }
            T1 = new_T1;
            L1 = new_L1;

            QVector<int> new_T2, new_L2;
            for (int i = 0; i < T2.count(); i++) {
                if (!inds2_to_remove.contains(i)) {
                    new_T2 << T2[i];
                    new_L2 << L2[i];
                }
            }
            T2 = new_T2;
            L2 = new_L2;
        }
    }

    //The rest are unclassified
    for (int ii = 0; ii < L1.count(); ii++) {
        CM[(L1[ii] - 1) + (K1 + 1) * K2]++;
    }
    for (int ii = 0; ii < L2.count(); ii++) {
        CM[K1 + (K1 + 1) * (L2[ii] - 1)]++;
    }

    Mda output;
    output.allocate(K1 + 1, K2 + 1);
    for (int k1 = 1; k1 <= K1 + 1; k1++) {
        for (int k2 = 1; k2 <= K2 + 1; k2++) {
            output.setValue(CM[(k1 - 1) + (K1 + 1) * (k2 - 1)], k1 - 1, k2 - 1);
        }
    }

    return output;
}

QVector<int> indexlist(const QVector<int>& T2, int t1, int offset, int& ptr2)
{
    // find T1 such that abs(T1-t1)<off, where t1 is a scalar. Update ptr2 which gives the rough
    // center value of this index.
    int N = T2.count();
    int i = ptr2;
    while ((T2.value(i) >= t1 - offset) && (i >= 0))
        i--; // go down until T2's too early
    int j = ptr2;
    while ((T2.value(j) <= t1 + offset) && (j < N))
        j++; // go up until T2's too late
    QVector<int> ret;
    for (int ii = i; ii <= j; ii++) {
        int t2val = T2.value(ii);
        if ((ii >= 0) && (ii < N) && (t1 - offset <= t2val) && (t2val <= t1 + offset))
            ret << ii;
    }
    if (ret.count() > 0) {
        ptr2 = ret[0]; //update the pointer
    }
    return ret;
}
