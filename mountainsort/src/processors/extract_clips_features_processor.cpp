/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/5/2016
*******************************************************/

#include "extract_clips_features_processor.h"
#include "extract_clips.h"
#include "pca.h"

class extract_clips_features_ProcessorPrivate {
public:
    extract_clips_features_Processor* q;
};

extract_clips_features_Processor::extract_clips_features_Processor()
{
    d = new extract_clips_features_ProcessorPrivate;
    d->q = this;

    this->setName("extract_clips_features");
    this->setVersion("0.1");
    this->setInputFileParameters("timeseries", "firings");
    this->setOutputFileParameters("features");
    this->setRequiredParameters("clip_size", "num_features");
}

extract_clips_features_Processor::~extract_clips_features_Processor()
{
    delete d;
}

bool extract_clips_features_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool extract_clips_features_Processor::run(const QMap<QString, QVariant>& params)
{
    QString timeseries_path = params["timeseries"].toString();
    QString firings_path = params["firings"].toString();
    QString features_path = params["features"].toString();
    int clip_size = params["clip_size"].toInt();
    int num_features = params["num_features"].toInt();

    qDebug() << __FUNCTION__ << __FILE__ << __LINE__;
    DiskReadMda X(timeseries_path);
    DiskReadMda F(firings_path);
    QVector<double> times;
    //QVector<int> labels;
    for (long i = 0; i < F.N2(); i++) {
        times << F.value(1, i);
        //labels << (int)F.value(2,i);
    }
    qDebug() << __FUNCTION__ << __FILE__ << __LINE__;
    Mda clips = extract_clips(X, times, clip_size);
    qDebug() << __FUNCTION__ << __FILE__ << __LINE__;
    long M = clips.N1();
    long T = clips.N2();
    long L = clips.N3();
    qDebug() << __FUNCTION__ << __FILE__ << __LINE__;
    Mda clips_reshaped(M * T, L);
    qDebug() << __FUNCTION__ << __FILE__ << __LINE__;
    long NNN = M * T * L;
    qDebug() << __FUNCTION__ << __FILE__ << __LINE__;
    for (long iii = 0; iii < NNN; iii++) {
        clips_reshaped.set(clips.get(iii), iii);
    }
    qDebug() << __FUNCTION__ << __FILE__ << __LINE__;
    Mda CC, FF, sigma;
    qDebug() << __FUNCTION__ << __FILE__ << __LINE__;
    pca(CC, FF, sigma, clips_reshaped, num_features);
    qDebug() << __FUNCTION__ << __FILE__ << __LINE__;
    qDebug() << __FUNCTION__ << __FILE__ << __LINE__ << FF.N1() << FF.N2() << features_path;
    return FF.write32(features_path);

    //return extract_clips_features(timeseries_path,firings_path,features_path,clip_size,num_features);
}
