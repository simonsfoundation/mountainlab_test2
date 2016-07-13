/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/13/2016
*******************************************************/

#include "filter_events.h"
#include "mda.h"

bool filter_events(QString firings_path, QString firings_out_path, filter_events_opts opts)
{
    Mda F1;
    F1.read(firings_path);

    QList<long> inds_to_use;
    for (long i = 0; i < F1.N2(); i++) {
        double outlier_score = F1.value(4, i);
        double detectability_score = F1.value(5, i);
        bool use_it = false;
        if ((!opts.outlier_threshold) || (outlier_score <= opts.outlier_threshold)) {
            if ((!opts.detectability_threshold) || (detectability_score >= opts.detectability_threshold)) {
                use_it = true;
            }
        }
        if (use_it)
            inds_to_use << i;
    }
    Mda F2;
    F2.allocate(F1.N1(), inds_to_use.count());

    for (long i = 0; i < inds_to_use.count(); i++) {
        for (int j = 0; j < F1.N1(); j++) {
            F2.setValue(F1.value(j, inds_to_use[i]), j, i);
        }
    }

    int pct = (int)((F2.N2() * 1.0 / F1.N2()) * 100);
    printf("Using %ld of %ld events (%d%%).\n", F2.N2(), F1.N2(), pct);

    F2.write64(firings_out_path);

    return true;
}
