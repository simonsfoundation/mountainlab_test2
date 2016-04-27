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

static void fill_mda(Mda& X, const int M, const int N) {
	X.allocate(M,N);
	for (int n=0; n<N; n++)
		for (int m=0; m<M; m++)
			X.set(1+sin(m+sin(n)),m,n);
}

void TestMda::testDiskWrite() {
	Mda X1, Y1;

	QTemporaryFile tempFile;
	QVERIFY(tempFile.open());

	fill_mda(X1, 4, 10);

	QVERIFY(X1.write64(tempFile.fileName()));
	QVERIFY(Y1.read(tempFile.fileName()));
	QVERIFY(qFuzzyIsNull(max_difference(X1, Y1)));
}

void TestMda::benchmarkAllocate() {
	Mda m;
	QBENCHMARK(m.allocate(512, 512));
}

void TestMda::benchmarkDiskWrite() {
	QTemporaryFile tempFile;
	QVERIFY(tempFile.open());

	Mda X;
	fill_mda(X, 32, 32);
	QBENCHMARK(X.write32(tempFile.fileName()));
}

void TestMda::benchmarkDiskRead() {
	QTemporaryFile tempFile;
	QVERIFY(tempFile.open());

	Mda X;
	fill_mda(X, 64, 64);
	X.write32(tempFile.fileName());
	QBENCHMARK(X.read(tempFile.fileName()));
}
