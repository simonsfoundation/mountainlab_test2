#include "quantize_processor.h"

#include <QFile>
#include <QFileInfo>
#include <mda.h>
#include <mda32.h>
#include "mlcommon.h"

class quantize_ProcessorPrivate {
public:
    quantize_Processor* q;
};

quantize_Processor::quantize_Processor()
{
    d = new quantize_ProcessorPrivate;
    d->q = this;

    this->setName("quantize");
    this->setVersion("0.1");
    this->setInputFileParameters("input");
    this->setOutputFileParameters("output");
    this->setRequiredParameters("output_format"); //eg "int16", "uint16", "uint8"
}

quantize_Processor::~quantize_Processor()
{
    delete d;
}

bool quantize_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool quantize(QString input_path, QString output_path, QString output_format);
bool quantize_Processor::run(const QMap<QString, QVariant>& params)
{
    QString input_path = params["input"].toString();
    QString output_path = params["output"].toString();
    QString output_format = params["output_format"].toString();
    if (!QFile::exists(input_path)) {
        qWarning() << "File does not exist:" << input_path;
        return false;
    }
    return quantize(input_path, output_path, output_format);
}

bool quantize(QString input_path, QString output_path, QString output_format)
{
    /// TODO, do this in chunks
    Mda X(input_path);
    Mda X_scaled(X.N1(), X.N2(), X.N3());
    double minval = X.minimum();
    double maxval = X.maximum();
    double maxabs = qMax(qAbs(minval), qAbs(maxval));
    if (maxabs == 0)
        maxabs = 1;
    double dynamic_max = 0;
    if (output_format == "uint8")
        dynamic_max = 256 - 1;
    else if (output_format == "uint16")
        dynamic_max = 256L * 256 - 1;
    else if (output_format == "uint32")
        dynamic_max = 256L * 256 * 256 * 256 - 1;
    else if (output_format == "int16")
        dynamic_max = 256L * 256 / 2 - 1;
    else if (output_format == "int32")
        dynamic_max = 256L * 256 * 256 * 256 / 2 - 1;
    else {
        qWarning() << "Unexpected output format: " + output_format;
        return false;
    }
    double scale_factor = dynamic_max / maxabs;
    bool is_unsigned = (output_format.startsWith("u"));
    double* ptr1 = X.dataPtr();
    double* ptr2 = X_scaled.dataPtr();
    long N = X.totalSize();
    for (long i = 0; i < N; i++) {
        ptr2[i] = ptr1[i] * scale_factor;
    }
    if (is_unsigned) {
        for (long i = 0; i < N; i++) {
            if (ptr2[i] < 0)
                ptr2[i] = 0;
        }
    }
    if (output_format == "uint8")
        return X_scaled.write8(output_path);
    else if (output_format == "uint16")
        return X_scaled.write16ui(output_path);
    else if (output_format == "uint32")
        return X_scaled.write32ui(output_path);
    else if (output_format == "int16")
        return X_scaled.write16i(output_path);
    else if (output_format == "int32")
        return X_scaled.write32i(output_path);
    else {
        qWarning() << "Unexpected output format*: " + output_format;
        return false;
    }
}

MSProcessorTestResults quantize_Processor::runTest(int test_number, const QMap<QString, QVariant>& file_params)
{
    MSProcessorTestResults results;
    QMap<QString, QVariant> params = file_params;

    QString input_path = params["input"].toString();
    QString output_path = params["output"].toString();

    if ((0 <= test_number) && (test_number < 1)) {
        results.test_exists = true;

        results.params = params;
        results.params["output_format"] = "uint8";

        Mda X(3, 3);
        for (int j = 0; j < X.totalSize(); j++) {
            X.setValue(j * 100, j);
        }
        X.write64(input_path);
        if (!this->run(results.params)) {
            results.success = false;
            results.error_message = "Error running processor";
            return results;
        }
        Mda Y(output_path);
        if ((Y.minimum() != 0) || (Y.maximum() == 255)) {
            results.success = false;
            results.error_message = "Wrong min or max";
            return results;
        }
        if ((Y.N1() != X.N1()) || (Y.N2() != X.N2())) {
            results.success = false;
            results.error_message = "Wrong dimensions";
            return results;
        }
        results.success = true;
    }
    else {
        results.test_exists = false;
    }

    return results;
}
