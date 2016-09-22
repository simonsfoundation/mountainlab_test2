#include "expfilter.h"
#include <math.h>
#include "mdaio.h"

int expfilter(FILE *infile,FILE *outfile,int lowpass,float tau) {
	long i,j,i1;

	struct MDAIO_HEADER HH;
	mda_read_header(&HH,infile);
	if (!HH.data_type) return 0;

	int32_t M=HH.dims[0];
	int32_t N=HH.dims[1];

	struct MDAIO_HEADER HH2;
	mda_copy_header(&HH2,&HH);
	HH2.data_type=MDAIO_TYPE_FLOAT32;

	mda_write_header(&HH2,outfile);

	if (M<=0) return 0;
	if (N<=0) return 0;

	float alpha=exp(-1.0/tau);

	int chunk_size=100000;
	float *aa=(float *)malloc(sizeof(float)*M);
	float *buffer_in=(float *)malloc(sizeof(float)*chunk_size*M);
	float *buffer_out=(float *)malloc(sizeof(float)*chunk_size*M);
	for (i=0; i<N; i+=chunk_size) {
		printf("i = %ld / %d (%d percent)\n",i,N,(int)((i*1.0/N)*100));

		int K=chunk_size;
		if (i+K>N) K=N-i;
		mda_read_float32(buffer_in,&HH,K*M,infile);

		for (i1=0; i1<K; i1++) {
			for (j=0; j<M; j++) {
				if (i1==0) {
					aa[j]=buffer_in[j+i1*M];
				}
				else {
					aa[j]=buffer_in[j+i1*M]*alpha+aa[j]*(1-alpha);
				}

				if (lowpass) buffer_out[j+i1*M]=aa[j];
				else buffer_out[j+i1*M]=buffer_in[j+i1*M]-aa[j];
			}
		}

		mda_write_float32(buffer_out,&HH2,K*M,outfile);
	}
	free(aa);
	free(buffer_in);
	free(buffer_out);

	return 1;
}
