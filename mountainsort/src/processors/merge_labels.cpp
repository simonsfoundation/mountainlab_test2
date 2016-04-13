/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/13/2016
*******************************************************/

#include "merge_labels.h"
#include "diskreadmda.h"
#include "msmisc.h"
#include "compute_templates_0.h"

bool merge_labels(QString timeseries_path, QString firings_path, QString firings_out_path, merge_labels_opts opts)
{
    DiskReadMda X(timeseries_path);
    DiskReadMda F(firings_path);

    QList<double> times;
    QList<int> labels;
    for (long i=0; i<F.N2(); i++) {
        times << F.value(1,i);
        labels << (int)F.value(2,i);
    }

    Mda templates0=compute_templates_0(X,times,labels,opts.clip_size);
    int M=templates0.N1();
    int T=templates0.N2();
    int K=templates0.N3();

    Mda comparison_matrix(K,K);
    for (int k1=0; k1<K; k1++) {
        for (int k2=0; k2<K; k2++) {
            //if (about_the_sample_template(M,T,templates0.dataPtr(0,0,k1),templates0.dataPtr(0,0,k2))) {
            //    comparison_matrix.setValue(1,k1,k2);
            //}
        }
    }

}
