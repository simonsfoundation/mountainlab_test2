/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/4/2016
*******************************************************/

#include "mv_firings_filter.h"

#include <mda.h>
#include "msmisc.h"
#include <math.h>

void define_shells(QVector<double>& shell_mins, QVector<double>& shell_maxs, QVector<double>& clip_peaks, double shell_increment, int min_shell_count);

bool mv_firings_filter(const QString& firings_path, const QString& firings_out_path, const QString& original_cluster_numbers_path, const mv_firings_filter_opts& opts)
{

    double shell_width = opts.shell_width;
    int min_per_shell = opts.min_per_shell;

    Mda firings_original(firings_path);
    Mda original_cluster_numbers;
    Mda firings_split;

    QVector<int> labels;
    QVector<double> peaks;
    for (int n = 0; n < firings_original.N2(); n++) {
        float peak = firings_original.value(3, n);
        labels << (int)firings_original.value(2, n);
        peaks << peak;
    }

    int K = compute_max(labels);

    if (opts.use_shell_split) {
        QVector<int> nums;
        QList<float> mins;
        QList<float> maxs;
        for (int k = 1; k <= K; k++) {
            QVector<double> peaks_k;
            for (int n = 0; n < labels.count(); n++) {
                if (labels[n] == k) {
                    peaks_k << peaks[n];
                }
            }
            QVector<double> shell_mins, shell_maxs;
            shell_mins.clear();
            shell_maxs.clear();
            define_shells(shell_mins, shell_maxs, peaks_k, shell_width, min_per_shell);
            for (int ii = 0; ii < shell_mins.count(); ii++) {
                nums << k;
                mins << shell_mins[ii];
                maxs << shell_maxs[ii];
            }
        }

        firings_split.allocate(firings_original.N1(), firings_original.N2());
        for (int i2 = 0; i2 < firings_split.N2(); i2++) {
            for (int i1 = 0; i1 < firings_split.N1(); i1++) {
                if (i1 != 2) { //don't set the labels!
                    firings_split.setValue(firings_original.value(i1, i2), i1, i2);
                }
            }
        }

        int KK = nums.count();
        original_cluster_numbers.allocate(1, KK + 1);
        for (int kk = 1; kk <= KK; kk++) {
            int k = nums[kk - 1];
            float min0 = mins[kk - 1];
            float max0 = maxs[kk - 1];
            original_cluster_numbers.set(k, 0, kk);
            for (int n = 0; n < labels.count(); n++) {
                if (labels[n] == k) {
                    if ((min0 <= peaks[n]) && (peaks[n] < max0)) {
                        firings_split.setValue(kk, 2, n);
                    }
                }
            }
        }

    } else {
        firings_split = firings_original;
        original_cluster_numbers.allocate(1, K + 1);
        for (int k = 1; k <= K; k++)
            original_cluster_numbers.set(k, 0, k);
    }

    Mda firings_out;
    if (opts.use_event_filter) {
        double min_amplitude = opts.min_amplitude;
        double min_detectablity_score = opts.min_detectability_score;
        double max_outlier_score = opts.max_outlier_score;

        QVector<int> inds;
        for (int i = 0; i < firings_split.N2(); i++) {
            if (fabs(firings_split.value(3, i)) >= min_amplitude) {
                if (fabs(firings_split.value(5, i)) >= min_detectablity_score) {
                    if (max_outlier_score) {
                        if (firings_split.value(4, i) <= max_outlier_score) {
                            inds << i;
                        }
                    } else {
                        inds << i;
                    }
                }
            }
        }

        int N2 = inds.count();
        firings_out.allocate(firings_split.N1(), N2);
        for (int i = 0; i < N2; i++) {
            for (int j = 0; j < firings_split.N1(); j++) {
                firings_out.setValue(firings_split.value(j, inds[i]), j, i); //speed this up?
            }
        }
    } else
        firings_out = firings_split;

    firings_out.write64(firings_out_path);
    original_cluster_numbers.write64(original_cluster_numbers_path);
    return true;
}

void define_shells(QVector<double>& shell_mins, QVector<double>& shell_maxs, QVector<double>& clip_peaks, double shell_increment, int min_shell_count)
{
    //positives
    double max_clip_peaks = compute_max(clip_peaks);
    QVector<double> shell_mins_pos;
    QVector<double> shell_maxs_pos;
    {
        int num_bins = 1;
        while (shell_increment * num_bins <= max_clip_peaks)
            num_bins++;
        num_bins++;
        long counts[num_bins];
        for (int b = 0; b < num_bins; b++)
            counts[b] = 0;
        long tot = 0;
        for (int i = 0; i < clip_peaks.count(); i++) {
            if (clip_peaks[i] > 0) {
                int b = (int)(clip_peaks[i] / shell_increment);
                if (b < num_bins)
                    counts[b]++;
                else
                    qWarning() << "Unexpected problem" << __FILE__ << __LINE__;
                tot++;
            }
        }
        int min_b = 0;
        int max_b = 0;
        int count_in = counts[0];
        while (min_b < num_bins) {
            if ((count_in >= min_shell_count) && (tot - count_in >= min_shell_count)) {
                shell_mins_pos << min_b* shell_increment;
                shell_maxs_pos << (max_b + 1) * shell_increment;
                min_b = max_b + 1;
                max_b = max_b + 1;
                tot -= count_in;
                if (min_b < num_bins)
                    count_in = counts[min_b];
                else
                    count_in = 0;
            } else {
                max_b++;
                if (max_b < num_bins) {
                    count_in += counts[max_b];
                } else {
                    if (count_in > 0) {
                        shell_mins_pos << min_b* shell_increment;
                        shell_maxs_pos << (max_b + 1) * shell_increment;
                    }
                    break;
                }
            }
        }
    }

    //negatives
    double min_clip_peaks = compute_min(clip_peaks);
    QVector<double> shell_mins_neg;
    QVector<double> shell_maxs_neg;
    {
        int num_bins = 1;
        while (shell_increment * num_bins <= -min_clip_peaks)
            num_bins++;
        num_bins++;
        long counts[num_bins];
        for (int b = 0; b < num_bins; b++)
            counts[b] = 0;
        long tot = 0;
        for (int i = 0; i < clip_peaks.count(); i++) {
            if (clip_peaks[i] < 0) {
                int b = (int)(-clip_peaks[i] / shell_increment);
                if (b < num_bins)
                    counts[b]++;
                else
                    qWarning() << "Unexpected problem" << __FILE__ << __LINE__;
                tot++;
            }
        }
        int min_b = 0;
        int max_b = 0;
        int count_in = counts[0];
        while (min_b < num_bins) {
            if ((count_in >= min_shell_count) && (tot - count_in >= min_shell_count)) {
                shell_mins_neg << min_b* shell_increment;
                shell_maxs_neg << (max_b + 1) * shell_increment;
                min_b = max_b + 1;
                max_b = max_b + 1;
                tot -= count_in;
                if (min_b < num_bins)
                    count_in = counts[min_b];
                else
                    count_in = 0;
            } else {
                max_b++;
                if (max_b < num_bins) {
                    count_in += counts[max_b];
                } else {
                    if (count_in > 0) {
                        shell_mins_neg << min_b* shell_increment;
                        shell_maxs_neg << (max_b + 1) * shell_increment;
                    }
                    break;
                }
            }
        }
    }

    //combine
    for (int i = shell_mins_neg.count() - 1; i >= 0; i--) {
        shell_maxs << -shell_mins_neg[i];
        shell_mins << -shell_maxs_neg[i];
    }
    for (int i = 0; i < shell_mins_pos.count(); i++) {
        shell_mins << shell_mins_pos[i];
        shell_maxs << shell_maxs_pos[i];
    }
}
