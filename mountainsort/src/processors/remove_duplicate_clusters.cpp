#include "remove_duplicate_clusters.h"
#include "mda.h"
#include <QList>
#include <stdio.h>
#include "get_sort_indices.h"

typedef QList<long> IntList;
bool remove_duplicate_clusters(const QString &firings_path,const QString &firings_out_path,int max_dt,float overlap_threshold) {
	Mda F; F.read(firings_path);
    QList<double> times;
    QList<int> labels;

	int L=F.N2();
	int K=1;
	for (int ii=0; ii<L; ii++) {
        double time0=F.value(1,ii);
		int label0=(int)F.value(2,ii);
		times << time0;
		labels << label0;
		if (label0>K) K=label0;
	}

	printf("Initializing output...\n");
	QList<IntList> out;
	IntList empty_list;
	for (int k1=1; k1<=K; k1++) {
		for (int k2=1; k2<=K; k2++) {
			out << empty_list;
		}
	}

	printf("Sorting times/labels...\n");
    QList<long> indices=get_sort_indices(times);
    QList<double> times2;
    QList<int> labels2;
	for (int i=0; i<indices.count(); i++) {
		times2 << times[indices[i]];
		labels2 << labels[indices[i]];
	}
	times=times2; labels=labels2;

	printf("Counting...\n");
	int counts[K+1];
	for (int ii=0; ii<K+1; ii++) counts[ii]=0;
	for (int ii=0; ii<times.count(); ii++) counts[labels[ii]]++;

	printf("Cross counting");
	int cross_counts[K+1][K+1];
	for (int k1=0; k1<=K; k1++) {
		for (int k2=0; k2<=K; k2++) {
			cross_counts[k1][k2]=0;
		}
	}
	int i1=0;
	for (int i2=0; i2<L; i2++) {
		while ((i1+1<L)&&(times[i1]<times[i2]-max_dt)) i1++;
		int k2=labels[i2];
		if (k2>=1) {
			for (int jj=i1; jj<i2; jj++) {
				int k1=labels[jj];
				if (k1>=1) {
					cross_counts[k1][k2]++;
					cross_counts[k2][k1]++;
				}
			}
		}
	}

	printf("Finding labels to use...\n");
	bool to_use[K+1]; to_use[0]=true;
	for (int k=1; k<=K; k++) to_use[k]=true;
	for (int k1=1; k1<=K; k1++) {
		for (int k2=k1+1; k2<=K; k2++) {
			if (counts[k1]>=counts[k2]) {
				if (cross_counts[k1][k2]>=overlap_threshold*counts[k2]) {
					to_use[k2]=false;
				}
			}
			else {
				if (cross_counts[k1][k2]>=overlap_threshold*counts[k1]) {
					to_use[k1]=false;
				}
			}
		}
	}

	printf("Creating labels map...\n");
	int labels_map[K+1]; labels_map[0]=0;
	int k0=1;
	for (int k=1; k<=K; k++) {
		if (to_use[k]) {
			labels_map[k]=k0;
			k0++;
		}
		else {
			printf("Removing label %d\n",k);
			labels_map[k]=0;
		}
	}

	printf("Creating output firings...\n");
	long L_new=0;
	for (int i=0; i<L; i++) {
		int label0=(int)F.value(2,i);
		if (to_use[label0]) L_new++;
	}
	Mda F2;
	F2.allocate(F.N1(),L_new);
	int j=0;
	for (int i=0; i<L; i++) {
		int label0=(int)F.value(2,i);
		if (to_use[label0]) {
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
