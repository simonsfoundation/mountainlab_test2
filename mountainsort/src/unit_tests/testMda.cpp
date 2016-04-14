#include <qmath.h>
#include <QTemporaryFile>
#include "testMda.h"
#include "mda.h"
#include "diskwritemda.h"

static double max_difference(Mda &X, Mda &Y) {
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

void TestMda::testDiskWrite() {
	Mda X1, Y1;

	QTemporaryFile tempFile;
	QVERIFY(tempFile.open());

	int M=4;
	int N=10;
	X1.allocate(M,N);
	for (int n=0; n<N; n++) {
		for (int m=0; m<M; m++) {
			X1.set(1+sin(m+sin(n)),m,n);
		}
	}

	QVERIFY(X1.write64(tempFile.fileName()));
	QVERIFY(Y1.read(tempFile.fileName()));
	QVERIFY(qFuzzyIsNull(max_difference(X1, Y1)));
}
