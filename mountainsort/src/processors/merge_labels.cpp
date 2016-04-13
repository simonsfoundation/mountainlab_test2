/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/13/2016
*******************************************************/

#include "merge_labels.h"
#include "diskreadmda.h"
#include "msmisc.h"
#include "compute_templates_0.h"
#include <math.h>

double compute_template_similarity(int M,int T,double *template1,double *template2);

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

    Mda similarity_matrix(K,K);
    for (int k1=0; k1<K; k1++) {
        for (int k2=0; k2<K; k2++) {
            double similarity_score=compute_template_similarity(M,T,templates0.dataPtr(0,0,k1),templates0.dataPtr(0,0,k2));
            similarity_matrix.setValue(similarity_score,k1,k2);
        }
    }

    Mda merge_matrix(K,K);
    for (int i=0; i<K; i++) {
        for (int j=0; j<K; j++) {
            if (similarity_matrix.value(i,j)>opts.merge_threshold) { //merge with an earlier guy
                merge_matrix.setValue(1,i,j);
            }
        }
    }

    //make it transitive
    bool something_changed=true;
    while (something_changed) {
        something_changed=false;
        for (int i=0; i<K; i++) {
            for (int j=0; j<K; j++) {
                for (int k=0; k<K; k++) {
                    if ((merge_matrix.value(i,j))&&(merge_matrix.value(j,k))&&(!merge_matrix.value(i,k))) {
                        merge_matrix.setValue(1,i,k);
                        something_changed=true;
                    }
                }
            }
        }
    }

    int merge_map[K];
    //merge with the smallest label
    for (int i=0; i<K; i++) merge_map[i]=i;
    for (int i=0; i<K; i++) {
        for (int j=0; j<i; j++) {
            if (merge_matrix.value(i,j)) {
                if (merge_map[j]<merge_map[i]) {
                    merge_map[i]=merge_map[j];
                }
            }
        }
    }

    int use_it[K];
    for (int i=0; i<K; i++) use_it[i]=0;
    for (int i=0; i<K; i++) use_it[merge_map[i]]=1;
    int merge_map2[K];
    int k0=0;
    for (int i=0; i<K; i++) {
        if (use_it[i]) {
            merge_map2[i]=k0;
            k0++;
        }
        else merge_map2[i]=-1;
    }

    for (int i=0; i<K; i++) {
        printf("%d <--- %d\n",merge_map2[merge_map[i]]+1,i+1);
    }

    printf("Using %d of %d clusters.\n",k0,K);

    for (long i=0; i<labels.count(); i++) {
        int new_label=merge_map2[merge_map[labels[i]-1]]+1;
        labels[i]=new_label;
    }

    Mda firings_out(F.N1(),F.N2());
    for (long i=0; i<F.N2(); i++) {
        for (int j=0; j<F.N1(); j++) {
            firings_out.setValue(F.value(j,i),j,i);
        }
        firings_out.setValue(labels[i],2,i);
    }

    firings_out.write64(firings_out_path);

    return true;
}

double compute_template_similarity(int M,int T,double *template1,double *template2) {
    int N=M*T;
    double S1=0,S2=0;
    for (int i=0; i<N; i++) {
        S1+=template1[i];
        S2+=template2[i];
    }
    double mean1=S1/N;
    double mean2=S2/N;
    double S11=0,S22=0,S12=0;
    for (int i=0; i<N; i++) {
        double v1=template1[i]-mean1;
        double v2=template2[i]-mean2;
        S11+=v1*v1;
        S12+=v1*v2;
        S22+=v2*v2;
    }
    if ((S11==0)||(S22==0)) return 0;
    return S12/sqrt(S11*S22);
}
