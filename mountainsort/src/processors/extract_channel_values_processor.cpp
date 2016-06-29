#include "extract_channel_values_processor.h"
#include "diskreadmda.h"

class extract_channel_values_ProcessorPrivate {
public:
    extract_channel_values_Processor* q;
};

extract_channel_values_Processor::extract_channel_values_Processor()
{
    d = new extract_channel_values_ProcessorPrivate;
    d->q = this;

    this->setName("extract_channel_values");
    this->setVersion("0.1");
    this->setInputFileParameters("timeseries", "firings");
    this->setOutputFileParameters("values");
    this->setRequiredParameters("channels");
}

extract_channel_values_Processor::~extract_channel_values_Processor()
{
    delete d;
}

bool extract_channel_values_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool extract_channel_values_Processor::run(const QMap<QString, QVariant>& params)
{
    QString timeseries_path = params["timeseries"].toString();
    QString firings_path = params["firings"].toString();
    QString values_path = params["values"].toString();
    QStringList channels_strlist = params["channels"].toString().split(",");
    QVector<int> channels;
    for (int i = 0; i < channels_strlist.count(); i++) {
        channels << channels_strlist[i].toInt();
    }
    int num_channels = channels.count();
    if (!num_channels) {
        qWarning() << "number of channels is zero.";
        return false;
    }
    DiskReadMda X(timeseries_path);
    DiskReadMda F(firings_path);
    Mda values(num_channels, F.N2());
    for (long i = 0; i < F.N2(); i++) {
        long t0 = (long)F.value(1, i);
        for (int j = 0; j < channels.count(); j++) {
            double val = X.value(channels[j] - 1, t0);
            values.setValue(val, j, i);
        }
    }
    return values.write32(values_path);
}
