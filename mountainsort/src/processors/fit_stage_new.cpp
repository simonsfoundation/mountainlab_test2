/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/21/2016
*******************************************************/

#include "fit_stage.h"
#include <QList>
#include "mlcommon.h"
#include "mlcommon.h"
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
    //Just for timing things
    QTime timer_total;
    timer_total.start();
    QMap<QString, long> elapsed_times;

    //The timeseries data and the dimensions
    DiskReadMda X(timeseries_path);
    long M = X.N1();
    long N = X.N2();
    int T = opts.clip_size;

    //Read in the firings array
    Mda firingsA;
    firingsA.read(firings_path);
    Mda firings = sort_firings_by_time(firingsA);

    //These are the options for splitting into shells
    Define_Shells_Opts define_shells_opts;
    define_shells_opts.min_shell_size = opts.min_shell_size;
    define_shells_opts.shell_increment = opts.shell_increment;

    //Here we split into shells to handle amplitude variation
    Mda firings_split = split_into_shells(firings, define_shells_opts);

    //These are the templates corresponding to the sub-clusters (after shell splitting)
    Mda templates = compute_templates_0(X, firings_split, T); //MxTxK (wrong: MxNxK)

    //L is the number of events. Accumulate vectors of times and labels for convenience
    long L = firings.N2();
    QVector<double> times;
    QVector<int> labels;
    for (long j = 0; j < L; j++) {
        times << firings.value(1, j);
        labels << (int)firings_split.value(2, j); //these are labels of sub-clusters
    }

    //Now we do the processing in chunks
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
            Mda chunk; //this will be the chunk we are working on
            Mda local_templates; //just a local copy of the templates
            QVector<double> local_times; //the times that fall in this time range
            QVector<int> local_labels; //the corresponding labels
            QList<long> local_inds; //the corresponding event indices
            fit_stage_opts local_opts;
#pragma omp critical(lock1)
            {
                //build the variables above
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
            //Our real task is to decide which of these events to keep. Those will be stored in local_inds_to_use
            //"Local" means this chunk in this thread
            QList<long> local_inds_to_use;
            {
                QTime timer;
                timer.start();
                //This is the main kernel operation!!
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
    int M = X.N1(); //the number of dimensions
    int T = opts.clip_size; //the clip size
    int Tmid = (int)((T + 1) / 2) - 1; //the center timepoint in a clip (zero-indexed)
    long L = times.count(); //number of events we are looking at
    int K = MLCompute::max<int>(labels); //the maximum label number

    //compute the L2-norms of the templates ahead of time
    QVector<double> template_norms;
    template_norms << 0;
    for (int k = 1; k <= K; k++) {
        template_norms << MLCompute::norm(M * T, templates.dataPtr(0, 0, k - 1));
    }

    //keep passing through the data until nothing changes anymore
    bool something_changed = true;
    QVector<int> all_to_use; //a vector of 0's and 1's telling which events should be used
    for (long i = 0; i < L; i++)
        all_to_use << 0; //start out using none
    int num_passes = 0;
    //while ((something_changed)&&(num_passes<2)) {
    while (something_changed) {
        num_passes++;
        QVector<double> scores_to_try;
        QVector<double> times_to_try;
        QVector<int> labels_to_try;
        QList<long> inds_to_try; //indices of the events to try on this pass
        //QVector<double> template_norms_to_try;
        for (long i = 0; i < L; i++) { //loop through the events
            if (all_to_use[i] == 0) { //if we are not yet using it...
                double t0 = times[i];
                int k0 = labels[i];
                if (k0 > 0) { //make sure we have a positive label (don't know why we wouldn't)
                    long tt = (long)(t0 - Tmid + 0.5); //start time of clip
                    double score0 = 0;
                    if ((tt >= 0) && (tt + T <= X.N2())) { //make sure we are in range
                        //The score will be how much something like the L2-norm is decreased
                        score0 = compute_score(M * T, X.dataPtr(0, tt), templates.dataPtr(0, 0, k0 - 1));
                    }
                    /*
                    if (score0 < template_norms[k0] * template_norms[k0] * 0.1)
                        score0 = 0; //the norm of the improvement needs to be at least 0.5 times the norm of the template
                        */

                    //the score needs to be at least as large as neglogprior in order to accept the spike
                    double neglogprior = 30;
                    if (score0 > neglogprior) {
                        //we are not committing to using this event yet... see below for next step
                        scores_to_try << score0;
                        times_to_try << t0;
                        labels_to_try << k0;
                        inds_to_try << i;
                    }
                    else {
                        //means we definitely aren't using it (so we will never get here again)
                        all_to_use[i] = -1; //signals not to try again
                    }
                }
            }
        }
        //Look at those events to try and see if we should use them
        QVector<int> to_use = find_events_to_use(times_to_try, scores_to_try, opts);

        //for all those we are going to "use", we want to subtract out the corresponding templates from the timeseries data
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
