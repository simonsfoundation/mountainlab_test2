/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/21/2016
*******************************************************/

#include "fit_stage.h"
#include <QList>
#include "msmisc.h"
#include "diskreadmda.h"
#include <QTime>
#include <math.h>
#include "compute_templates_0.h"
#include "compute_detectability_scores.h"
#include "get_sort_indices.h"
#include "msprefs.h"
#include "omp.h"

QList<long> fit_stage_kernel(Mda& X, Mda& templates, QVector<double>& times, QVector<int>& labels, const fit_stage_opts& opts);

bool fit_stage_new(const QString& timeseries_path, const QString& firings_path, const QString& firings_out_path, const fit_stage_opts& opts)
{
    QTime timer_total;
    timer_total.start();
    QMap<QString, long> elapsed_times;

    DiskReadMda X(timeseries_path);
    long M = X.N1();
    long N = X.N2();
    int T = opts.clip_size;

    Mda firingsA;
    firingsA.read(firings_path);
    Mda firings = sort_firings_by_time(firingsA);

    Define_Shells_Opts define_shells_opts;
    define_shells_opts.min_shell_size = opts.min_shell_size;
    define_shells_opts.shell_increment = opts.shell_increment;
    Mda firings_split = split_into_shells(firings, define_shells_opts);

    Mda templates = compute_templates_0(X, firings_split, T); //MxNxK

    long L = firings.N2();
    QVector<double> times;
    QVector<int> labels;
    for (long j = 0; j < L; j++) {
        times << firings.value(1, j);
        labels << (int)firings_split.value(2, j);
    }

    long chunk_size = PROCESSING_CHUNK_SIZE;
    long overlap_size = PROCESSING_CHUNK_OVERLAP_SIZE;
    if (N < PROCESSING_CHUNK_SIZE) {
        chunk_size = N;
        overlap_size = 0;
    }

    QList<long> inds_to_use;

    {
        QTime timer_status;
        timer_status.start();
        long num_timepoints_handled = 0;
#pragma omp parallel for
        for (long timepoint = 0; timepoint < N; timepoint += chunk_size) {
            QMap<QString, long> elapsed_times_local;
            Mda chunk;
            Mda local_templates;
            QVector<double> local_times;
            QVector<int> local_labels;
            QList<long> local_inds;
            fit_stage_opts local_opts;
#pragma omp critical(lock1)
            {
                QTime timer;
                timer.start();
                local_templates = templates;
                local_opts = opts;
                X.readChunk(chunk, 0, timepoint - overlap_size, M, chunk_size + 2 * overlap_size);
                elapsed_times["readChunk"] += timer.elapsed();
                timer.start();
                for (long jj = 0; jj < L; jj++) {
                    if ((timepoint - overlap_size <= times[jj]) && (times[jj] < timepoint - overlap_size + chunk_size + 2 * overlap_size)) {
                        local_times << times[jj] - (timepoint - overlap_size);
                        local_labels << labels[jj];
                        local_inds << jj;
                    }
                }
                elapsed_times["set_local_data"] += timer.elapsed();
            }
            QList<long> local_inds_to_use;
            {
                QTime timer;
                timer.start();
                local_inds_to_use = fit_stage_kernel(chunk, local_templates, local_times, local_labels, local_opts);
                elapsed_times_local["fit_stage_kernel"] += timer.elapsed();
            }
#pragma omp critical(lock1)
            {
                elapsed_times["fit_stage_kernel"] += elapsed_times_local["fit_stage_kernel"];
                {
                    QTime timer;
                    timer.start();
                    for (long ii = 0; ii < local_inds_to_use.count(); ii++) {
                        long ind0 = local_inds[local_inds_to_use[ii]];
                        double t0 = times[ind0];
                        if ((timepoint <= t0) && (t0 < timepoint + chunk_size)) {
                            inds_to_use << ind0;
                        }
                    }
                    elapsed_times_local["get_local_data"] += timer.elapsed();
                }

                num_timepoints_handled += qMin(chunk_size, N - timepoint);
                if ((timer_status.elapsed() > 1000) || (num_timepoints_handled == N) || (timepoint == 0)) {
                    printf("%ld/%ld (%d%%) - Elapsed(s): RC:%g, SLD:%g, KERNEL:%g, GLD:%g, Total:%g, %d threads\n",
                        num_timepoints_handled, N,
                        (int)(num_timepoints_handled * 1.0 / N * 100),
                        elapsed_times["readChunk"] * 1.0 / 1000,
                        elapsed_times["set_local_data"] * 1.0 / 1000,
                        elapsed_times["fit_stage_kernel"] * 1.0 / 1000,
                        elapsed_times["get_local_data"] * 1.0 / 1000,
                        timer_total.elapsed() * 1.0 / 1000,
                        omp_get_num_threads());
                    timer_status.restart();
                }
            }
        }
    }

    qSort(inds_to_use);
    long num_to_use = inds_to_use.count();
    if (times.count()) {
        printf("using %ld/%ld events (%g%%)\n", num_to_use, (long)times.count(), num_to_use * 100.0 / times.count());
    }
    Mda firings_out(firings.N1(), num_to_use);
    for (long i = 0; i < num_to_use; i++) {
        for (int j = 0; j < firings.N1(); j++) {
            firings_out.set(firings.get(j, inds_to_use[i]), j, i);
        }
    }

    firings_out.write64(firings_out_path);

    return true;
}

QList<long> fit_stage_kernel(Mda& X, Mda& templates, QVector<double>& times, QVector<int>& labels, const fit_stage_opts& opts)
{
    int M = X.N1();
    int T = opts.clip_size;
    int Tmid = (int)((T + 1) / 2) - 1;
    long L = times.count();
    int K = compute_max(labels);

    QVector<double> template_norms;
    template_norms << 0;
    for (int k = 1; k <= K; k++) {
        template_norms << compute_norm(M * T, templates.dataPtr(0, 0, k - 1));
    }

    bool something_changed = true;
    QVector<int> all_to_use;
    for (long i = 0; i < L; i++)
        all_to_use << 0;
    int num_passes = 0;
    //while ((something_changed)&&(num_passes<2)) {
    while (something_changed) {
        num_passes++;
        QVector<double> scores_to_try;
        QVector<double> times_to_try;
        QVector<int> labels_to_try;
        QList<long> inds_to_try; //indices of the events to try on this pass
        //QVector<double> template_norms_to_try;
        for (long i = 0; i < L; i++) {
            if (all_to_use[i] == 0) {
                double t0 = times[i];
                int k0 = labels[i];
                if (k0 > 0) {
                    long tt = (long)(t0 - Tmid + 0.5);
                    double score0 = 0;
                    if ((tt >= 0) && (tt + T <= X.N2())) {
                        score0 = compute_score(M * T, X.dataPtr(0, tt), templates.dataPtr(0, 0, k0 - 1));
                    }
                    /*
                    if (score0 < template_norms[k0] * template_norms[k0] * 0.1)
                        score0 = 0; //the norm of the improvement needs to be at least 0.5 times the norm of the template
                        */

                    double neglogprior = 30;
                    if (score0 > neglogprior) {
                        scores_to_try << score0;
                        times_to_try << t0;
                        labels_to_try << k0;
                        inds_to_try << i;
                    }
                    else {
                        all_to_use[i] = -1; //means we definitely aren't using it (so we will never get here again)
                    }
                }
            }
        }
        QVector<int> to_use = find_events_to_use(times_to_try, scores_to_try, opts);

        something_changed = false;
        long num_added = 0;
        for (long i = 0; i < to_use.count(); i++) {
            if (to_use[i] == 1) {
                something_changed = true;
                num_added++;
                long tt = (long)(times_to_try[i] - Tmid + 0.5);
                subtract_scaled_template(M * T, X.dataPtr(0, tt), templates.dataPtr(0, 0, labels_to_try[i] - 1));
                all_to_use[inds_to_try[i]] = 1;
            }
        }
        //printf("pass %d: added %ld events\n", num_passes, num_added);
    }

    QList<long> inds_to_use;
    for (long i = 0; i < L; i++) {
        if (all_to_use[i] == 1)
            inds_to_use << i;
    }

    return inds_to_use;
}
