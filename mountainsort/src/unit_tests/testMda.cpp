#include <qmath.h>
#include <QTemporaryFile>
#include <QDebug>
#include <cstring>
#include "testMda.h"
#include "mda.h"
#include "diskwritemda.h"

static double max_difference(const Mda& X, const Mda& Y) {
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

static int num_dims(const long dims[6]) {
	for (int i=5 ; i>=0 ; --i)
		if (dims[i] > 1)
			return i+1;
	return 2;
}

static bool verify_sizes(const Mda& m, const long dims[6], const QString& message) {
	bool result = true;
	long totalSize = 1;
	for (int k=0 ; k<6 ; ++k) {
		totalSize *= dims[k];
		if (m.size(k) != dims[k])
			result = false;
	}
	result = result && (m.N1() == dims[0]);
	result = result && (m.N2() == dims[1]);
	result = result && (m.N3() == dims[2]);
	result = result && (m.N4() == dims[3]);
	result = result && (m.N5() == dims[4]);
	result = result && (m.N6() == dims[5]);
	result = result && (m.ndims() == num_dims(dims));
	result = result && (m.totalSize() == totalSize);
	if (!result) {
		qDebug() << message << ", verify_sizes() failed for dimensions ";
		for (int k=0 ; k<6 ; ++k)
			qDebug() << dims[k];
	}
	return result;
}

void TestMda::testSizes() {
	qsrand(time(0));
	for (int i=0 ; i<100 ; ++i) {
		long dims[6];
		long totalSize = 1;
		for (int k = 0 ; k<6 ; ++k) {
			dims[k] = qrand() % 10 + 1;
			totalSize *= dims[k];
		}
		const Mda m(dims[0], dims[1], dims[2], dims[3], dims[4], dims[5]);
		QVERIFY(verify_sizes(m, dims, "Mda(int[])"));
		const Mda clone(m);
		QVERIFY(verify_sizes(clone, dims, "Mda(const Mda&)"));
		Mda assigned;
		assigned = m;
		QVERIFY(verify_sizes(assigned, dims, "operator = "));
		QTemporaryFile file;
		QVERIFY(file.open());
		QVERIFY(m.write32(file.fileName()));
		const Mda fromFile(file.fileName());
		QVERIFY(verify_sizes(fromFile, dims, "Mda(const QString&)"));
		Mda fromRead;
		fromRead.read(file.fileName());
		QVERIFY(verify_sizes(fromRead, dims, "Mda.read(const QString&)"));
		Mda fromAllocate;
		fromAllocate.allocate(dims[0], dims[1], dims[2], dims[3], dims[4], dims[5]);
		QVERIFY(verify_sizes(fromAllocate, dims, "Mda.allocate()"));
		Mda reshaped(dims[0], dims[1], dims[2], dims[3], dims[4], dims[5]);
		reshaped.reshape(totalSize, 1);
		QCOMPARE(reshaped.N1(), totalSize);
		QCOMPARE(reshaped.N2(), 1L);
		QCOMPARE(reshaped.N3(), 1L);
		QCOMPARE(reshaped.N4(), 1L);
		QCOMPARE(reshaped.N5(), 1L);
		QCOMPARE(reshaped.totalSize(), totalSize);
	}
}

static bool test_values() {
	bool result = true;
	const int M = 16;
	const int N = 32;
	Mda originalData;
	fill_mda(originalData, M, N);
	Mda toFill(originalData.N1(), originalData.N2());
	Mda toFillSafe(originalData.N1(), originalData.N2());
	for (int n=0 ; n<N ; ++n) {
		for (int m=0 ; m<M ; ++m) {
			toFill.set(originalData.get(m, n), m, n);
			if (!qFuzzyCompare(toFill.get(m, n), originalData.get(m, n)))
				result = false;
			toFillSafe.setValue(originalData.value(m, n), m, n);
			if (!qFuzzyCompare(toFill.value(m, n), originalData.value(m, n)))
				result = false;
		}
	}
	result = result && (0 == std::memcmp(toFill.dataPtr(), originalData.dataPtr(), toFill.totalSize() * sizeof(toFill.dataPtr()[0])));
	result = result && (0 == std::memcmp(toFill.dataPtr(), toFillSafe.dataPtr(), toFill.totalSize() * sizeof(toFill.dataPtr()[0])));
	return result;
}

void TestMda::testValues() {
	QVERIFY(test_values());
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

void TestMda::benchmarkValues() {
	QBENCHMARK(test_values());
}
