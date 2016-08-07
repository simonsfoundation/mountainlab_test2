#include <QString>
#include <QtTest>
#include "mda/mda.h"
#include "mda/mdareader.h"
#include <QTemporaryFile>

class MdaReaderTest : public QObject
{
    Q_OBJECT

public:
    MdaReaderTest();

private Q_SLOTS:
    void testCase1();
};

MdaReaderTest::MdaReaderTest()
{
}

void MdaReaderTest::testCase1()
{
    Mda mda(2, 2, 3);
    QVector<double> dat = {
        42.1,
        -2.0,
        16.5,
        0,
        123,
        -16,
        32.2,
        98.4,
        -127.92
    };
    for(int i = 0; i < dat.size(); ++i)
        mda.set(dat.at(i), i);
    mda.write64("/tmp/abc.mda");

    MdaReader reader("/tmp/abc.mda");
    Mda mda2 = reader.read();
    QCOMPARE(mda.ndims(), mda2.ndims());

    for(int i = 0; i < 6; ++i) {
        QCOMPARE(mda.size(i), mda2.size(i));
    }
    for(int i = 0; i < dat.size(); ++i) {
        QCOMPARE(mda.get(i), mda2.get(i));
    }

    MdaWriter writer("/tmp/abc2.mda", "mda");
    QVERIFY(writer.write(mda2));
    mda.read("/tmp/abc2.mda");
    QCOMPARE(mda.ndims(), mda2.ndims());
    for(int i = 0; i < 6; ++i) {
        QCOMPARE(mda.size(i), mda2.size(i));
    }
    for(int i = 0; i < dat.size(); ++i) {
        QCOMPARE(mda.get(i), mda2.get(i));
    }
}

QTEST_APPLESS_MAIN(MdaReaderTest)

#include "tst_mdareadertest.moc"
