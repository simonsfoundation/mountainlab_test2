#include "jisotonic.h"
#include <malloc.h>

void jisotonic(int N,double *BB,double *MSE,double *AA,double *WW) {
	if (N<1) return;

	double *unweightedcount=(double *)malloc(sizeof(double)*N);
	double *count=(double *)malloc(sizeof(double)*N);
	double *sum=(double *)malloc(sizeof(double)*N);
	double *sumsqr=(double *)malloc(sizeof(double)*N);
	int last_index=-1;

	last_index++;
	unweightedcount[last_index]=1;
	double w0=1; if (WW) w0=WW[0];
	count[last_index]=w0;
	sum[last_index]=AA[0]*w0;
	sumsqr[last_index]=AA[0]*AA[0]*w0;
	MSE[0]=0;

	for (int j=1; j<N; j++) {
		last_index++;
		unweightedcount[last_index]=1;
		w0=1; if (WW) w0=WW[j];
		count[last_index]=w0;
		sum[last_index]=AA[j]*w0;
		sumsqr[last_index]=AA[j]*AA[j]*w0;
		MSE[j]=MSE[j-1];
		while (true) {
			if (last_index<=0) break;
			if (sum[last_index-1]/count[last_index-1]<sum[last_index]/count[last_index]) {
				break;
			}
			else {
				double prevMSE=sumsqr[last_index-1]-sum[last_index-1]*sum[last_index-1]/count[last_index-1];
				prevMSE+=sumsqr[last_index]-sum[last_index]*sum[last_index]/count[last_index];
				unweightedcount[last_index-1]+=unweightedcount[last_index];
				count[last_index-1]+=count[last_index];
				sum[last_index-1]+=sum[last_index];
				sumsqr[last_index-1]+=sumsqr[last_index];
				double newMSE=sumsqr[last_index-1]-sum[last_index-1]*sum[last_index-1]/count[last_index-1];
				MSE[j]+=newMSE-prevMSE;
				last_index--;
			}
		}
	}

	int ii=0;
	for (int k=0; k<=last_index; k++) {
		for (int cc=0; cc<unweightedcount[k]; cc++) {
			BB[ii+cc]=sum[k]/count[k];
		}
		ii+=unweightedcount[k];
	}

	free(unweightedcount);
	free(count);
	free(sum);
	free(sumsqr);
}
