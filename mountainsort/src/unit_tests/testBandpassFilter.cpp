#include "testBandpassFilter.h"
#include "bandpass_filter_processor.h"
#include "mda.h"
#include <QDir>
#include <QCoreApplication>
#include <QTemporaryFile>

static QDir unitTestDataDir()
{
    QDir d(QCoreApplication::applicationDirPath());
    d.cdUp();
    d.cd("src/unit_tests/data/");
    return d;
}

static QString testFilePath(const QString& baseName)
{
    static QDir dir = unitTestDataDir();
    if (dir.exists() && dir.exists(baseName))
        return dir.absoluteFilePath(baseName);
    qWarning() << "TEST FILE MISSING: " << baseName << dir.absolutePath();
    return QString();
}

void TestBandpassFilter::testGroundTruth()
{
    const QString groundTruth = testFilePath("ground_truth_bandpass_filter.mda");
    QVERIFY(QFile::exists(groundTruth));
    QFile output(QString("bandpass_test_%1.mda").arg(QDateTime::currentDateTime().toString("dMyyhhmmss")));
    QVERIFY(output.open(QIODevice::WriteOnly));
    QMap<QString, QVariant> params;
    params["timeseries"] = testFilePath("input_bandpass_filter.mda");
    params["timeseries_out"] = output.fileName();
    params["samplerate"] = 20000;
    params["freq_min"] = 300;
    params["freq_max"] = 10000;
    params["freq_wid"] = 1000; // ahb added
    bandpass_filter_Processor proc;
    QVERIFY(proc.run(params));
    output.close();
    Mda outputMda(output.fileName());
    Mda groundTruthMda(groundTruth);
    QCOMPARE(outputMda.N1(), groundTruthMda.N1());
    QCOMPARE(outputMda.N2(), groundTruthMda.N2());
    QCOMPARE(outputMda.totalSize(), groundTruthMda.totalSize());
    for (int i = 0; i < outputMda.N1(); ++i) {
        for (int j = 0; j < outputMda.N2(); ++j) {
            QCOMPARE(outputMda.get(i, j), groundTruthMda.get(i, j));
        }
    }
    output.remove();
}
