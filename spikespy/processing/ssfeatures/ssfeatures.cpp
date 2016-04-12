#include "ssfeatures.h"
#include <QStringList>
#include <math.h>
#include <QDebug>
#include "mdaio.h"
#include "array2d.h"
#include "pcasolver.h"

int ssfeatures(FILE *infile,FILE *outfile,const QMap<QString,QVariant> &params) {
	int nfeatures=params["nfeatures"].toInt();
	int niterations=params["niterations"].toInt();

	//get the input file header
	MDAIO_HEADER HH_infile;
	if (!mda_read_header(&HH_infile,infile)) return 0;
	int32_t M=HH_infile.dims[0];
	int32_t T=HH_infile.dims[1];
	int32_t N=HH_infile.dims[2];
	if (M<=0) return 0;
	if (T<=0) return 0;
	if (N<=0) return 0;

	Array2D X;
	X.allocate(M*T,N);
	float *inbuf=(float *)malloc(sizeof(float)*M*T);
	for (int i=0; i<N; i++) {
		mda_read_float32(inbuf,&HH_infile,M*T,infile);
		for (int j=0; j<M*T; j++) {
			X.setValue(inbuf[j],j,i);
		}
	}
	free(inbuf);

	PCASolver SS;
	SS.setVectors(X);
	SS.setNumIterations(niterations);
	SS.setComponentCount(nfeatures);
	SS.solve();
	Array2D features=SS.coefficients();

	//write the output header
	MDAIO_HEADER HH_outfile;
	mda_copy_header(&HH_outfile,&HH_infile);
	HH_outfile.num_dims=2;
	HH_outfile.dims[0]=nfeatures;
	HH_outfile.dims[1]=N;
	HH_outfile.dims[2]=1;
	HH_outfile.data_type=MDAIO_TYPE_FLOAT32;
	mda_write_header(&HH_outfile,outfile);

	float *outbuf=(float *)malloc(sizeof(float)*nfeatures);
	for (int i=0; i<N; i++) {
		for (int j=0; j<nfeatures; j++) {
			outbuf[j]=features.value(j,i);
		}
		mda_write_float32(outbuf,&HH_outfile,nfeatures,outfile);
	}
	free(outbuf);

	return 1;
}
