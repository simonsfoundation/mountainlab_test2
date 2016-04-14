#ifndef TESTMDA_H
#define TESTMDA_H

#include <QtTest/QTest>

class TestMda: public QObject {
	Q_OBJECT
private slots:
	void testDiskWrite();
};

#endif // TESTMDA_H
