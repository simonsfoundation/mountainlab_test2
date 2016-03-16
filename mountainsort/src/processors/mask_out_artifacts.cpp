#include "mask_out_artifacts.h"
#include "diskreadmda.h"
#include "diskwritemda.h"
#include <math.h>
#include "msmisc.h"

bool mask_out_artifacts(const QString &raw_in_path, const QString &raw_out_path, double threshold, int interval_size)
{
	DiskReadMda X(raw_in_path);
	long M=X.N1();
	long N=X.N2()

	Mda norms(M,N/interval_size);
	for (long i=0; i<N/interval_size; i++) {
		long timepoint=i*interval_size;
		Mda chunk;
		X.readChunk(chunk,0,timepoint,M,interval_size);
		for (int m=0; m<M; m++) {
			double sumsqr=0;
			for (int aa=0; aa<interval_size; aa++) {
				sumsqr+=chunk.value(m,aa)*chunk.value(m,aa);
			}
			norms.set(sqrt(sumsqr),m,i);
		}
	}

	Mda use_it(M,N/interval_size);
	for (long i=0; i<use_it.totalSize(); i++) use_it.set(1,i);
	for (int m=0; m<M; m++) {
		QList<double> vals;
		for (long i=0; i<norms.N2(); i++) {
			vals << norms.get(m,i);
		}
		double sigma0=compute_stdev(vals);
		for (int i=0; i<norms.N2(); i++) {
			if (norms.value(m,i)>sigma0*threshold) {
				use_it.setValue(0,m,i-1);
				use_it.setValue(0,m,i);
				use_it.setValue(0,m,i+1);
			}
		}
	}

	DiskWriteMda Y(); Y.open(MDAIO_TYPE_FLOAT32,raw_out_path,M,N);
	for (long i=0; i<N/interval_size; i++) {
		long timepoint=i*interval_size;
		Mda chunk;
		X.readChunk(chunk,0,timepoint,M,interval_size);
		for (int m=0; m<M; m++) {
			if (use_it.value(m,i)) {

			}
		}
	}
	Y.close();

	return true;

}
