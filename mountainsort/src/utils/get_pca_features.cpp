#include "get_pca_features.h"
#include "mda.h"
#include "eigenvalue_decomposition.h"
#include "get_sort_indices.h"

#include <QTime>

bool get_pca_features(long M, long N, int num_features, double *features_out, double *X_in, long num_representatives)
{
    long increment=1;
    if (num_representatives) increment=qMax(1L,N/num_representatives);
    QTime timer; timer.start();
    Mda XXt(M,M);
    double *XXt_ptr=XXt.dataPtr();
    for (long i=0; i<N; i+=increment) {
        double *tmp=&X_in[i*M];
        long aa=0;
        for (long m2=0; m2<M; m2++) {
            for (long m1=0; m1<M; m1++) {
                XXt_ptr[aa]+=tmp[m1]*tmp[m2];
                aa++;
            }
        }
    }

    Mda U;
    Mda S;
    eigenvalue_decomposition_sym(U,S,XXt);
    QList<double> eigenvals;
    for (int i=0; i<S.totalSize(); i++) eigenvals << S.get(i);
    QList<long> inds=get_sort_indices(eigenvals);

    Mda FF(num_features,N);
    long aa=0;
    for (long i=0; i<N; i++) {
        double *tmp=&X_in[i*M];
        for (int j=0; j<num_features; j++) {
            if (inds.count()-1-j>=0) {
                long k=inds.value(inds.count()-1-j);
                double val=0;
                for (long m=0; m<M; m++) {
                    val+=U.get(m,k)*tmp[m];
                }
                FF.set(val,aa);
            }
            aa++;
        }
    }

    for (long i=0; i<FF.totalSize(); i++) {
        features_out[i]=FF.get(i);
    }

    return true;
}
