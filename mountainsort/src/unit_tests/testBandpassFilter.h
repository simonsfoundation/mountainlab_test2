#ifndef TESTBANDPASSFILTER_H
#define TESTBANDPASSFILTER_H

#include <QtTest/QTest>

class TestBandpassFilter : public QObject {
    Q_OBJECT
private slots:
    void testGroundTruth();
};

#endif // TESTBANDPASSFILTER_H
