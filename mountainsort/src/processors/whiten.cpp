#include "whiten.h"
#include "diskreadmda.h"
#include "diskwritemda.h"
#include "mda.h"
#include "msprefs.h"
#include <math.h>
#include <QDebug>
#include "get_sort_indices.h"
#include "eigenvalue_decomposition.h"
#include "matrix_mda.h"

Mda get_whitening_matrix(Mda &COV);

bool whiten(const QString &input, const QString &output)
{
	DiskReadMda X(input);
	long M=X.N1();
	long N=X.N2();

	Mda COV(M,M);
	double *COVptr=COV.dataPtr();
	long chunk_size=PROCESSING_CHUNK_SIZE;
	if (N<PROCESSING_CHUNK_SIZE) {
		chunk_size=N;
	}

	{
		Mda chunk;
		long timepoint=0;
		while (timepoint<N) {
			X.getSubArray(chunk,0,timepoint,M,qMin(chunk_size,N-timepoint));
			double *chunkptr=chunk.dataPtr();
			for (long i=0; i<chunk.N2(); i++) {
				long aa=M*i;
				long bb=0;
				for (int m1=0; m1<M; m1++) {
					for (int m2=0; m2<M; m2++) {
						COVptr[bb]+=chunkptr[aa+m1]*chunkptr[aa+m2];
						bb++;
					}
				}
			}
			timepoint+=chunk_size;
		}
	}
	if (N>1) {
		for (int ii=0; ii<M*M; ii++) {
			COVptr[ii]/=(N-1);
		}
	}

	Mda AA=get_whitening_matrix(COV);
	double *AAptr=AA.dataPtr();

	DiskWriteMda Y;
	Y.open(MDAIO_TYPE_FLOAT32,output,M,N);
	{
		Mda chunk_in;
		long timepoint=0;
		while (timepoint<N) {
			X.getSubArray(chunk_in,0,timepoint,M,qMin(chunk_size,N-timepoint));
			double *chunk_in_ptr=chunk_in.dataPtr();
			Mda chunk_out(M,chunk_in.N2());
			double *chunk_out_ptr=chunk_out.dataPtr();
			for (long i=0; i<chunk_in.N2(); i++) {
				long aa=M*i;
				long bb=0;
				for (int m1=0; m1<M; m1++) {
					for (int m2=0; m2<M; m2++) {
						chunk_out_ptr[aa+m1]+=chunk_in_ptr[aa+m2]*AAptr[bb];
						bb++;
					}
				}
			}
			Y.writeSubArray(chunk_out,0,timepoint);
			timepoint+=chunk_size;
		}
	}
	Y.close();

	return true;

}

Mda get_whitening_matrix(Mda &COV) {
	int M=COV.N1();
	Mda U(M,M),S(1,M);
	eigenvalue_decomposition_sym(U,S,COV);
	Mda S2(M,M);
	for (int m=0; m<M; m++) {
		if (S.get(m)) {
			S2.set(1/sqrt(S.get(m)),m,m);
		}
	}
	Mda W=matrix_multiply(matrix_multiply(U,S2),matrix_transpose(U));
	return W;
}
