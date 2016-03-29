/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 3/29/2016
*******************************************************/

#include "compute_templates.h"
#include "msmisc.h"

Mda compute_templates(DiskReadMda &X, Mda &firings,int clip_size)
{
    int M=X.N1();
    int T=clip_size;
    long L=firings.N2();

    QList<long> times;
    QList<int> labels;
    for (long i=0; i<L; i++) {
        times << (long)(firings.value(1,i)+0.5);
        labels << (int)firings.value(2,i);
    }
    int K=compute_max(labels);

    int Tmid=(int)((T+1)/2)-1;

    Mda templates(M,T,K);
    QList<long> counts; for (int k=0; k<K; k++) counts << k;
    for (long i=0; i<L; i++) {
        int k=labels[i];
        long t0=times[i];
        if (k>=1) {
            for (int t=0; t<T; t++) {
                for (int m=0; m<M; m++) {
                    templates.set(templates.get(m,t,k-1)+X.value(m,t0+t-Tmid),m,t,k-1);
                }
            }
            counts[k-1]++;
        }
    }
    for (int k=0; k<K; k++) {
        for (int t=0; t<T; t++) {
            for (int m=0; m<M; m++) {
                if (counts[k]) {
                    templates.set(templates.get(m,t,k)/counts[k],m,t,k);
                }
            }
        }
    }

    return templates;
}
