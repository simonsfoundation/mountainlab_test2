#ifndef TESTMDAIO_H
#define TESTMDAIO_H

#include <QtTest/QTest>

class TestMdaIO : public QObject {
	Q_OBJECT
private slots:
	void testHeader();
	void testByte();
	void testFloat32();
	void testInt16();
	void testInt32();
	void testUInt16();
	void testFloat64();
};

#endif // TESTMDAIO_H
