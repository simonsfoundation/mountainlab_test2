/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/4/2016
*******************************************************/

#include "compute_templates.h"
#include "compute_templates_0.h"

bool compute_templates(const QString& timeseries_path, const QString& firings_path, const QString& templates_out_path, int clip_size)
{
    DiskReadMda X(timeseries_path);
    DiskReadMda firings(firings_path);
    QList<double> times;
    QList<int> labels;
    for (long i=0; i<firings.N2(); i++) {
        times << firings.value(1,i);
        labels << (int)firings.value(2,i);
    }
    Mda templates=compute_templates_0(X,times,labels,clip_size);
    templates.write32(templates_out_path);
    return true;
}
