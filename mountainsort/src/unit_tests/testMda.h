#ifndef TESTMDA_H
#define TESTMDA_H

#include <QtTest/QTest>

class TestMda : public QObject {
    Q_OBJECT
private slots:
    void testSizes();
    void testValues();
    void testDiskWrite();
    void benchmarkAllocate();
    void benchmarkDiskWrite();
    void benchmarkDiskRead();
    void benchmarkValues();
};

#endif // TESTMDA_H
