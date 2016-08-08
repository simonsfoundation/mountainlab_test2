#include <QString>
#include <QtTest>
#include "mda/mda.h"
#include "mda/mdareader.h"
#include <QTemporaryDir>

class MdaReaderTest : public QObject
{
    Q_OBJECT

public:
    MdaReaderTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void mda64();
    void mda32();
private:
    QTemporaryDir *m_dir;
};

MdaReaderTest::MdaReaderTest() : m_dir(0)
{
}

void MdaReaderTest::initTestCase()
{
    m_dir = new QTemporaryDir;
    if (!qgetenv("MDAREADERTEST_DEBUG_DIR").isEmpty()) {
        m_dir->setAutoRemove(false);
        qDebug() << "MdaReaderTest: test directory:" << m_dir->path();
    }
}

void MdaReaderTest::cleanupTestCase()
{
    delete m_dir;
    m_dir = 0;
}

void MdaReaderTest::mda64()
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
    mda.write64(m_dir->path()+"/abc.mda");

    MdaReader reader(m_dir->path()+"/abc.mda");
    Mda mda2 = reader.read();
    QCOMPARE(mda.ndims(), mda2.ndims());

    for(int i = 0; i < 6; ++i) {
        QCOMPARE(mda.size(i), mda2.size(i));
    }
    for(int i = 0; i < dat.size(); ++i) {
        QCOMPARE(mda.get(i), mda2.get(i));
    }

    MdaWriter writer(m_dir->path()+"/abc2.mda", "mda");
    QVERIFY(writer.write(mda2));
    mda.read(m_dir->path()+"/abc2.mda");
    QCOMPARE(mda.ndims(), mda2.ndims());
    for(int i = 0; i < 6; ++i) {
        QCOMPARE(mda.size(i), mda2.size(i));
    }
    for(int i = 0; i < dat.size(); ++i) {
        QCOMPARE(mda.get(i), mda2.get(i));
    }
    MdaWriter csvWriter(m_dir->path()+"/abc2.csv", "csv");
    QVERIFY(csvWriter.write(mda2));
}

void MdaReaderTest::mda32()
{
    Mda32 mda(2, 2, 3);
    QVector<float> dat = {
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
    mda.write32(m_dir->path()+"/32abc.mda");

    MdaReader reader(m_dir->path()+"/32abc.mda");
    Mda32 mda2 = reader.read32();
    QCOMPARE(mda.ndims(), mda2.ndims());

    for(int i = 0; i < 6; ++i) {
        QCOMPARE(mda.size(i), mda2.size(i));
    }
    for(int i = 0; i < dat.size(); ++i) {
        QCOMPARE(mda.get(i), mda2.get(i));
    }

    MdaWriter writer(m_dir->path()+"/32abc2.mda", "mda.float");
    QVERIFY(writer.write(mda2));
    mda.read(m_dir->path()+"/32abc2.mda");
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
