#include "copy_processor.h"

#include <QFile>
#include <QFileInfo>
#include "mlcommon.h"

class copy_ProcessorPrivate {
public:
    copy_Processor* q;
};

copy_Processor::copy_Processor()
{
    d = new copy_ProcessorPrivate;
    d->q = this;

    this->setName("copy");
    this->setVersion("0.1");
    this->setInputFileParameters("input");
    this->setOutputFileParameters("output");
}

copy_Processor::~copy_Processor()
{
    delete d;
}

bool copy_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool copy_Processor::run(const QMap<QString, QVariant>& params)
{
    QString input_path = params["input"].toString();
    QString output_path = params["output"].toString();
    if (!QFile::exists(input_path)) {
        qWarning() << "File does not exist:" << input_path;
        return false;
    }
    if (QFileInfo(input_path).canonicalFilePath() == QFileInfo(output_path).canonicalFilePath()) {
        return true;
    }
    if (QFile::exists(output_path)) {
        if (!QFile::remove(output_path)) {
            qWarning() << "Unable to remove file:" << output_path;
            return false;
        }
    }
    if (!QFile::copy(input_path, output_path)) {
        qWarning() << "Error copying file" << input_path << output_path;
        return false;
    }
    return true;
}

MSProcessorTestResults copy_Processor::runTest(int test_number, const QMap<QString, QVariant>& file_params)
{
    MSProcessorTestResults results;
    QMap<QString, QVariant> params = file_params;

    QString input_path = params["input"].toString();
    QString output_path = params["output"].toString();

    QStringList texts;
    texts << "This is a test\nof the copy processor";
    texts << "This is another test of the copy processor";

    if ((0 <= test_number) && (test_number < texts.count())) {
        results.test_exists = true;

        //set input parameters here

        results.params = params;
        QString txt = texts.value(test_number - 1);
        TextFile::write(input_path, txt);
        if (!this->run(params)) {
            results.success = false;
            results.error_message = "Error running processor";
            return results;
        }
        if (TextFile::read(output_path) != txt) {
            results.success = false;
            results.error_message = "Text does not match.";
            return results;
        }
        results.success = true;
    }
    else {
        results.test_exists = false;
    }

    return results;
}
