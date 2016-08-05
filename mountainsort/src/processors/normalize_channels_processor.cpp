#include "normalize_channels_processor.h"
#include "normalize_channels.h"

#include <mda.h>

class normalize_channels_ProcessorPrivate {
public:
    normalize_channels_Processor* q;
};

normalize_channels_Processor::normalize_channels_Processor()
{
    d = new normalize_channels_ProcessorPrivate;
    d->q = this;

    this->setName("normalize_channels");
    this->setVersion("0.1");
    this->setInputFileParameters("timeseries");
    this->setOutputFileParameters("timeseries_out");
}

normalize_channels_Processor::~normalize_channels_Processor()
{
    delete d;
}

bool normalize_channels_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool normalize_channels_Processor::run(const QMap<QString, QVariant>& params)
{
    QString input = params["timeseries"].toString();
    QString output = params["timeseries_out"].toString();
    return normalize_channels(input, output);
}

MSProcessorTestResults normalize_channels_Processor::runTest(int test_number, const QMap<QString, QVariant>& file_params)
{
    MSProcessorTestResults results;
    QMap<QString, QVariant> params = file_params;

    QString input = params["timeseries"].toString();
    QString output = params["timeseries_out"].toString();

    if ((0 <= test_number) && (test_number < 1)) {
        results.test_exists = true;

        //set input parameters here

        results.params = params;
        long M = 32;
        long N = 10000;
        Mda X(M, N);
        for (long n = 0; n < N; n++) {
            for (long m = 0; m < M; m++) {
                X.setValue(sin(n + sin(m)), m, n);
            }
        }
        X.write32(input);
        if (!this->run(params)) {
            results.success = false;
            results.error_message = "Error running processor";
            return results;
        }
        results.success = true;
    }
    else {
        results.test_exists = false;
    }

    return results;
}
