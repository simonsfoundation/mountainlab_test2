#ifndef TESTMDA_H
#define TESTMDA_H

#include <QtTest/QTest>

class TestMda: public QObject {
	Q_OBJECT
private slots:
	void testDiskWrite();
	void benchmarkAllocate();
	void benchmarkDiskWrite();
	void benchmarkDiskRead();
};

#endif // TESTMDA_H
