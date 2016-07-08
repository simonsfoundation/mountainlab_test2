#include "compute_outlier_scores.h"

#include "mda.h"
#include "diskreadmda.h"
#include "mlcommon.h"
#include <math.h>
#include "extract_clips.h"
#include "compute_detectability_scores.h"
#include "mlcommon.h"
#include "msmisc.h"

QVector<double> grab_sublist(const QVector<double>& X, const QVector<int>& inds);
QVector<double> compute_outlier_scores(Mda& clips, Mda& random_clips);

/*
Email exchange with Jason on 5/16/2016

On the one hand, yes, you can think of this is basically a z-scored distance to the template. But there are a few important differences, and this needs to be documented in the code and in the paper. Since this is fairly technical you may want to refer to this email during your presentation. See if people want more specifics.

1. The comparison template is for the average waveform in the subcluster (not the cluster). The cluster is the amplitude shell corresponding to the spike of interest.
2. (Important) The waveform (both template and spike) is weighted according to where the template is supported. This is very important in the case of nearby spikes, or simultaneous spikes on different channels. We don't want to classify a spike as an outlier just because another spike occurs nearby in time or space, but is not REALLY overlapping where the template is supported.
3. The noise distribution (or spread) is not estimated from the subcluster itself but from a random collection of clips, which are assumed to be noise. Again these clips are weighted by the
4. It's not the mahalanobis distance (which I believe is problematic). Instead it is the 1D z-score distance from the average sum-of-squares of the weighted events for the random collection of spikes. Yikes.

This can be improved, especially number 4, to be more based on theory. Eg, taking into account the covariance between timepoints and channels. Alex and I have discussed it.

 */

bool compute_outlier_scores(const QString& timeseries_path, const QString& firings_path, const QString& firings_out_path, const Compute_Outlier_Scores_Opts& opts)
{

    /// TODO speed this up, parallelize and print progress

    Mda firings;
    firings.read(firings_path);
    DiskReadMda X;
    X.setPath(timeseries_path);
    int N = X.N2();
    int L = firings.N2();
    QVector<double> times;
    QVector<int> labels;
    QVector<double> peaks;
    QVector<double> scores;
    for (int i = 0; i < L; i++) {
        times << firings.get(1, i);
        labels << (int)firings.get(2, i);
        peaks << firings.get(3, i);
        scores << 0;
    }

    int interval = (N / 5000);
    QVector<double> ttt;
    for (int i = 0; i < N; i += interval)
        ttt << i;
    Mda random_clips = extract_clips(X, ttt, opts.clip_size);

    Define_Shells_Opts opts2;
    opts2.min_shell_size = opts.min_shell_size;
    opts2.shell_increment = opts.shell_increment;

    int K = MLCompute::max<int>(labels);
    for (int k = 1; k <= K; k++) {
        QVector<int> inds_k;
        for (int i = 0; i < L; i++) {
            if (labels[i] == k)
                inds_k << i;
        }
        QVector<double> peaks_k = grab_sublist(peaks, inds_k);

        QList<Shell> shells = define_shells(peaks_k, opts2);
        for (int s = 0; s < shells.count(); s++) {
            QVector<int> inds_ks;
            for (int j = 0; j < shells[s].inds.count(); j++) {
                inds_ks << inds_k[shells[s].inds[j]];
            }
            QVector<double> times_ks = grab_sublist(times, inds_ks);
            Mda clips_ks = extract_clips(X, times_ks, opts.clip_size);
            QVector<double> scores_ks = compute_outlier_scores(clips_ks, random_clips);
            for (int j = 0; j < inds_ks.count(); j++) {
                scores[inds_ks[j]] = scores_ks[j];
            }
        }
    }

    int R = firings.N1();
    if (R < 5)
        R = 5;
    Mda firings2;
    firings2.allocate(R, L);
    for (int i = 0; i < L; i++) {
        for (int r = 0; r < R; r++) {
            firings2.set(firings.get(r, i), r, i);
        }
        firings2.set(scores[i], 4, i);
    }
    firings2.write64(firings_out_path);

    return true;
}
Mda get_template_weights(Mda& template0, int num_pix)
{
    int M = template0.N1();
    int T = template0.N2();
    Mda ret;
    ret.allocate(M, T);
    for (int m = 0; m < M; m++) {
        for (int t = 0; t < T; t++) {
            double val = 0;
            for (int dt = -T; dt <= T; dt++) {
                if ((0 <= t + dt) && (t + dt < T)) {
                    val += fabs(template0.get(m, t + dt)) * exp(-0.5 * dt * dt / (num_pix * num_pix));
                }
            }
            ret.set(val, m, t);
        }
    }
    return ret;
}

QVector<double> compute_outlier_scores(Mda& clips, Mda& random_clips)
{
    int M = clips.N1();
    int T = clips.N2();
    int L = clips.N3();
    int num_random_clips = random_clips.N3();
    Mda template0 = compute_mean_clip(clips);
    Mda weights = get_template_weights(template0, 6);
    Mda random_clips_weighted;
    random_clips_weighted.allocate(M, T, num_random_clips);
    {
        int aaa = 0;
        for (int i = 0; i < num_random_clips; i++) {
            int bbb = 0;
            for (int t = 0; t < T; t++) {
                for (int m = 0; m < M; m++) {
                    random_clips_weighted.set(random_clips.get(aaa) * weights.get(bbb), aaa);
                    aaa++;
                    bbb++;
                }
            }
        }
    }
    Mda clips_weighted;
    clips_weighted.allocate(M, T, L);
    {
        int aaa = 0;
        for (int i = 0; i < L; i++) {
            int bbb = 0;
            for (int t = 0; t < T; t++) {
                for (int m = 0; m < M; m++) {
                    clips_weighted.set(clips.get(aaa) * weights.get(bbb), aaa);
                    aaa++;
                    bbb++;
                }
            }
        }
    }
    Mda template_weighted = template0;
    for (int t = 0; t < T; t++) {
        for (int m = 0; m < M; m++) {
            template_weighted.set(template0.get(m, t) * weights.get(m, t), m, t);
        }
    }
    Mda diffs_weighted;
    diffs_weighted.allocate(M, T, L);
    {
        int aaa = 0;
        for (int i = 0; i < L; i++) {
            int bbb = 0;
            for (int t = 0; t < T; t++) {
                for (int m = 0; m < M; m++) {
                    diffs_weighted.set(clips_weighted.get(aaa) - template_weighted.get(bbb), aaa);
                    aaa++;
                    bbb++;
                }
            }
        }
    }
    QVector<double> vals1;
    {
        int aaa = 0;
        for (int i = 0; i < L; i++) {
            double val = 0;
            for (int t = 0; t < T; t++) {
                for (int m = 0; m < M; m++) {
                    val += diffs_weighted.get(aaa) * diffs_weighted.get(aaa);
                    aaa++;
                }
            }
            vals1 << val;
        }
    }
    QVector<double> vals2;
    {
        int aaa = 0;
        for (int i = 0; i < num_random_clips; i++) {
            double val = 0;
            for (int t = 0; t < T; t++) {
                for (int m = 0; m < M; m++) {
                    val += random_clips_weighted.get(aaa) * random_clips_weighted.get(aaa);
                    aaa++;
                }
            }
            vals2 << val;
        }
    }
    double mu0 = MLCompute::mean(vals2);
    double sigma0 = MLCompute::stdev(vals2);
    if (!sigma0)
        sigma0 = 1;
    QVector<double> scores;
    for (int i = 0; i < L; i++) {
        scores << (vals1[i] - mu0) / sigma0;
    }

    return scores;
}

QVector<double> grab_sublist(const QVector<double>& X, const QVector<int>& inds)
{
    QVector<double> ret;
    for (int i = 0; i < inds.count(); i++)
        ret << X[inds[i]];
    return ret;
}
