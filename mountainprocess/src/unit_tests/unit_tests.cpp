#include "unit_tests.h"

#include <mda.h>
#include <math.h>
#include <QCoreApplication>
#include <stdio.h>
#include <QDebug>
#include <diskwritemda.h>
#include <QTime>
#include "diskreadmda.h"

double max_difference(Mda &X,Mda &Y);

void run_all_unit_tests()
{
	Mda X1,Y1,Z1;

	QString path1="tmp1.mda";
	QString path2="tmp2.mda";
	QString path3="tmp3.mda";

	int M=4;
	int N=10;
	X1.allocate(M,N);
	for (int n=0; n<N; n++) {
		for (int m=0; m<M; m++) {
			X1.set(1+sin(m+sin(n)),m,n);
		}
	}

	X1.write64(path1);
	Y1.read(path1);
	printf("Max difference: %.16f\n",max_difference(X1,Y1));

	X1.write32(path2);
	Y1.read(path2);
	printf("Max difference: %.16f\n",max_difference(X1,Y1));

	DiskWriteMda X2;
	X2.open(MDAIO_TYPE_FLOAT64,path3,M,N);
    X2.writeChunk(X1,0,0);
	X2.close();
	Z1.read(path3);
	printf("Max difference: %.16f\n",max_difference(X1,Z1));

	/*
    double samplerate=10000;
	double freq_min=300;
	double freq_max=3000;
    bandpass_filter0(path1,path3,samplerate,freq_min,freq_max);
    */

	Mda tmp; tmp.read(path3);
	for (long i=0; i<tmp.totalSize(); i++) tmp.set(tmp.get(i)/2,i);
	printf("Max difference: %.16f\n",max_difference(X1,tmp));
}

double max_difference(Mda &X,Mda &Y) {
	if (X.N1()!=Y.N1()) return -1;
	if (X.N2()!=Y.N2()) return -1;
	if (X.N3()!=Y.N3()) return -1;
	if (X.N4()!=Y.N4()) return -1;
	if (X.N5()!=Y.N5()) return -1;
	if (X.N6()!=Y.N6()) return -1;
	double max_diff=0;
	for (long i=0; i<X.totalSize(); i++) {
		double diff=X.value(i)-Y.value(i);
		if (fabs(diff)>max_diff) max_diff=diff;
	}
	return max_diff;
}

void run_unit_test(const QString &test_name)
{
    Q_UNUSED(test_name)
    /*
	if (test_name=="eigenvalue_decomposition") {
		run_unit_test_eigenvalue_decomposition();
	}
	else if (test_name=="whiten") {
		run_unit_test_whiten();
	}
	else if (test_name=="isosplit") {
		run_unit_test_isosplit();
	}
    */
}



