#include "isocut.h"
#include "mda.h"
#include <math.h>
#include "jisotonic.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef QT_CORE_LIB
#include <QCoreApplication>
#include <QDebug>
#include <QTime>
#endif

double compute_ks(int N1,int N2,double *samples1,double *samples2);

void jisotonic_updown(int N,double *out,double *in,double *weights) {
	double *B1=(double *)malloc(sizeof(double)*N);
	double *MSE1=(double *)malloc(sizeof(double)*N);
	double *B2=(double *)malloc(sizeof(double)*N);
	double *MSE2=(double *)malloc(sizeof(double)*N);
	double *in_reversed=(double *)malloc(sizeof(double)*N);
    double *weights_reversed=0;

    for (int j=0; j<N; j++) {in_reversed[j]=in[N-1-j];}
	if (weights) {
        weights_reversed=(double *)malloc(sizeof(double)*N);
        for (int j=0; j<N; j++) {weights_reversed[j]=weights[N-1-j];}
	}
	jisotonic(N,B1,MSE1,in,weights);
	jisotonic(N,B2,MSE2,in_reversed,weights_reversed);
	for (int j=0; j<N; j++) MSE1[j]+=MSE2[N-1-j];
	double bestval=MSE1[0];
	int best_ind=0;
	for (int j=0; j<N; j++) {
		if (MSE1[j]<bestval) {
			bestval=MSE1[j];
			best_ind=j;
		}
	}
	jisotonic(best_ind+1,B1,MSE1,in,weights);
	jisotonic(N-best_ind,B2,MSE2,in_reversed,weights_reversed);
	for (int j=0; j<=best_ind; j++) out[j]=B1[j];
	for (int j=0; j<N-best_ind-1; j++) out[N-1-j]=B2[j];

	free(B1);
	free(MSE1);
	free(B2);
	free(MSE2);
	free(in_reversed);
	if (weights_reversed) free(weights_reversed);
}

void jisotonic_downup(int N,double *out,double *in,double *weights) {
	double *in_neg=(double *)malloc(sizeof(double)*N);

	for (int j=0; j<N; j++) in_neg[j]=-in[j];
	jisotonic_updown(N,out,in_neg,weights);
	for (int j=0; j<N; j++) out[j]=-out[j];

	free(in_neg);
}

void quick_sort (double *a, int n) {
	int i, j;
	double p,t;
	if (n < 2)
		return;
	p = a[n / 2];
	for (i = 0, j = n - 1;; i++, j--) {
		while (a[i] < p)
			i++;
		while (p < a[j])
			j--;
		if (i >= j)
			break;
		t = a[i];
		a[i] = a[j];
		a[j] = t;
	}
	quick_sort(a, i);
	quick_sort(&a[i], n - i);
}

void sort(int N,double *out,double *in) {
	for (int i=0; i<N; i++) out[i]=in[i];
	quick_sort(out,N);
//	QVector<double> in0(N);
//	for (int j=0; j<N; j++) in0[j]=in[j];
//	qSort(in0);
//	for (int j=0; j<N; j++) out[j]=in0[j];
}

//bool isocut(int N,double *cutpoint,double *samples_in) {
//    return isocut(N,cutpoint,samples_in,1.4,4);
//}

bool isocut(int N,double *cutpoint,double *samples_in,double threshold) {
    return isocut(N,cutpoint,samples_in,threshold,4);
}

bool isocut(int N,double *cutpoint,double *samples_in,double threshold,int minsize) {
	*cutpoint=0;
	double *samples=(double *)malloc(sizeof(double)*N);
	sort(N,samples,samples_in);
    
    int *N0s=(int *)malloc(sizeof(int)*(N*2+2)); //conservative malloc
    int num_N0s=0;
	for (int ii=2; ii<=floor(log2(N/2*1.0)); ii++) {
		N0s[num_N0s]=pow(2,ii);
        num_N0s++;
		N0s[num_N0s]=-pow(2,ii);
        num_N0s++;
	}
	N0s[num_N0s]=N;
    num_N0s++;

	bool found=false;
	for (int jj=0; (jj<num_N0s)&&(!found); jj++) {
		int N0=N0s[jj];
		int NN0=N0; if (N0<0) NN0=-N0;
		double *samples0=(double *)malloc(sizeof(double)*NN0);
		double *spacings0=(double *)malloc(sizeof(double)*(NN0-1));
		double *spacings0_fit=(double *)malloc(sizeof(double)*(NN0-1));
		double *samples0_fit=(double *)malloc(sizeof(double)*NN0);

		if (N0>0) {
			for (int ii=0; ii<NN0; ii++) {
				samples0[ii]=samples[ii];
			}
		}
		else {
			for (int ii=0; ii<NN0; ii++) {
				samples0[NN0-1-ii]=samples[N-1-ii];
			}
		}
		for (int ii=0; ii<NN0-1; ii++) {
			spacings0[ii]=samples0[ii+1]-samples0[ii];
		}
		jisotonic_downup(NN0-1,spacings0_fit,spacings0,0);
		samples0_fit[0]=samples0[0];
		for (int ii=1; ii<NN0; ii++) {
			samples0_fit[ii]=samples0_fit[ii-1]+spacings0_fit[ii-1];
		}
		double ks0=compute_ks(NN0,NN0,samples0,samples0_fit);
		ks0*=sqrt(NN0*1.0);
		if (ks0>=threshold) {
			for (int ii=0; ii<NN0-1; ii++) {
				if (spacings0_fit[ii]) spacings0[ii]=spacings0[ii]/spacings0_fit[ii];
			}
			jisotonic_updown(NN0-1,spacings0_fit,spacings0,0);
			if (NN0>=minsize*2) {
				int max_ind=minsize-1;
				double maxval=0;
				for (int ii=minsize-1; ii<=NN0-1-minsize; ii++) {
					if (spacings0_fit[ii]>maxval) {
						maxval=spacings0_fit[ii];
						max_ind=ii;
					}
				}
				*cutpoint=(samples0[max_ind]+samples0[max_ind+1])/2;
				found=true;
			}
		}

		free(samples0);
		free(spacings0);
		free(spacings0_fit);
		free(samples0_fit);
	}

    free(N0s);
	free(samples);

	return found;
}

double compute_ks(int N1,int N2,double *samples1,double *samples2) {
	double max_dist=0;
	int ii=0;
	for (int j=0; j<N2; j++) {
		while ((ii+1<N1)&&(samples1[ii+1]<=samples2[j])) ii++;
		double dist=fabs(j*1.0/N2-ii*1.0/N1);
		if (dist>max_dist) max_dist=dist;
	}
	return max_dist;
}
