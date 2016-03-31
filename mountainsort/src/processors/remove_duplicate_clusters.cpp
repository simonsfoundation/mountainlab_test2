#include "remove_duplicate_clusters.h"
#include "mda.h"
#include <QList>
#include <stdio.h>
#include "get_sort_indices.h"
#include "compute_templates.h"
#include <math.h>
#include "msmisc.h"

bool probably_the_same(Mda &templates,int ch1,int k1,int ch2,int k2,const remove_duplicate_clusters_Opts &opts);

typedef QList<long> IntList;
bool remove_duplicate_clusters(const QString &timeseries_path,const QString &firings_path,const QString &firings_out_path,const remove_duplicate_clusters_Opts &opts) {
    DiskReadMda X(timeseries_path);
	Mda F; F.read(firings_path);
    int T=opts.clip_size;
    int Tmid=(int)((T+1)/2)-1;

    QList<double> times;
    QList<int> labels;

	int L=F.N2();
    int K=0;
	for (int ii=0; ii<L; ii++) {
		int label0=(int)F.value(2,ii);
		if (label0>K) K=label0;
	}
    QList<int> cluster_channels;
    for (int k=0; k<K; k++) cluster_channels << 0;
    for (long i=0; i<L; i++) {
        int k=(int)F.value(2,i);
        int ch=(int)F.value(0,i);
        if (k>=1) {
            cluster_channels[k-1]=ch;
        }
    }

    QList<int> clusters_to_use;
    for (int k=0; k<K; k++) clusters_to_use << 1;

    printf("Computing templates...\n");
    Mda templates=compute_templates(X,F,opts.clip_size);
    printf("Comparing templates...\n");
    for (int k1=0; k1<K; k1++) {
        for (int k2=0; k2<K; k2++) {
            if ((clusters_to_use[k1])&&(clusters_to_use[k2])) {
                int ch1=cluster_channels[k1];
                int ch2=cluster_channels[k2];
                if (ch1!=ch2) {
                    if (probably_the_same(templates,ch1-1,k1,ch2-1,k2,opts)) {
                        if (fabs(templates.value(ch1,Tmid,k1))>fabs(templates.value(ch2,Tmid,k2))) {
                            clusters_to_use[k2]=0;
                        }
                        else {
                            clusters_to_use[k1]=0;
                        }
                    }
                }
            }
        }
    }
    printf("\n");

	printf("Creating labels map...\n");
	int labels_map[K+1]; labels_map[0]=0;
	int k0=1;
	for (int k=1; k<=K; k++) {
        if (clusters_to_use[k-1]) {
			labels_map[k]=k0;
			k0++;
		}
		else {
			printf("Removing label %d\n",k);
			labels_map[k]=0;
		}
	}

    //finish!!!

	printf("Creating output firings...\n");
	long L_new=0;
	for (int i=0; i<L; i++) {
		int label0=(int)F.value(2,i);
        if ((label0==0)||(clusters_to_use[label0])) L_new++;
	}
	Mda F2;
	F2.allocate(F.N1(),L_new);
	int j=0;
	for (int i=0; i<L; i++) {
		int label0=(int)F.value(2,i);
        if ((label0==0)||(clusters_to_use[label0])) {
			for (int a=0; a<F.N1(); a++) {
				F2.setValue(F.value(a,i),a,j);
			}
			F2.setValue(labels_map[label0],2,j);
			j++;
		}
	}

	printf("Writing...\n");
    F2.write64(firings_out_path);

	return true;
}

bool probably_the_same(Mda &templates,int ch1,int k1,int ch2,int k2,const remove_duplicate_clusters_Opts &opts) {
    Q_UNUSED(opts)
    int M=templates.N1();
    int T=templates.N2();
    int best_shift=0;
    double best_ip=0;
    double max11=0,max12=0;
    double max21=0,max22=0;
    for (int t=0; t<T; t++) {
        double val11=templates.value(ch1,t,k1),val12=templates.value(ch1,t,k2);
        double val21=templates.value(ch2,t,k1),val22=templates.value(ch2,t,k2);
        if (fabs(val11)>max11) max11=fabs(val11);
        if (fabs(val12)>max12) max12=fabs(val12);
        if (fabs(val21)>max21) max21=fabs(val21);
        if (fabs(val22)>max22) max22=fabs(val22);
    }
    if (max12<max11*0.5) return false;
    if (max21<max22*0.5) return false;
    for (int shift=-T/2; shift<=T/2; shift++) {
        double ip=0;
        for (int t=0; t<T; t++) {
            ip+=templates.value(ch1,t,k1)*templates.value(ch1,t-shift,k2);
            ip+=templates.value(ch2,t,k1)*templates.value(ch2,t-shift,k2);
        }
        if (ip>best_ip) {
            best_ip=ip;
            best_shift=shift;
        }
    }
    Mda resid(M,T);
    double *resid_ptr=resid.dataPtr();
    double *ptr1=templates.dataPtr(0,0,k1);
    double *ptr2=templates.dataPtr(0,0,k2);
    for (int t=0; t<T; t++) {
        for (int m=0; m<M; m++) {
            resid.setValue(templates.value(m,t,k1)-templates.value(m,t-best_shift,k2),m,t);
        }
    }
    double norm_resid=compute_norm(M*T,resid_ptr);
    double norm1=compute_norm(M*T,ptr1);
    double norm2=compute_norm(M*T,ptr2);
    if (norm_resid*norm_resid<norm1*norm1*0.5) {
        if (norm_resid*norm_resid<norm2*norm2*0.5) {
            return true;
        }
    }
    return false;
}

