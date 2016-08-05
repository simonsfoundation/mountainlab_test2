#include "msprocessor.h"

class MSProcessorPrivate {
public:
    MSProcessor* q;
    QString m_name;
    QString m_version;
    QString m_description;
    QStringList m_input_file_parameters;
    QStringList m_output_file_parameters;
    QStringList m_required_parameters;
    QStringList m_optional_parameters;
};

MSProcessor::MSProcessor()
{
    d = new MSProcessorPrivate;
    d->q = this;
}

MSProcessor::~MSProcessor()
{
    delete d;
}

QString MSProcessor::name()
{
    return d->m_name;
}

QString MSProcessor::version()
{
    return d->m_version;
}

QString MSProcessor::description()
{
    return d->m_description;
}

QStringList MSProcessor::inputFileParameters() const
{
    return d->m_input_file_parameters;
}

QStringList MSProcessor::outputFileParameters() const
{
    return d->m_output_file_parameters;
}

QStringList MSProcessor::requiredParameters() const
{
    return d->m_required_parameters;
}

QStringList MSProcessor::optionalParameters() const
{
    return d->m_optional_parameters;
}

//see info below
MSProcessorTestResults MSProcessor::runTest(int test_number, const QMap<QString, QVariant>& file_params)
{
    Q_UNUSED(test_number)
    Q_UNUSED(file_params)
    MSProcessorTestResults results;
    results.test_exists = false;
    return results;
}

void MSProcessor::setName(const QString& name)
{
    d->m_name = name;
}

void MSProcessor::setVersion(const QString& version)
{
    d->m_version = version;
}

void MSProcessor::setDescription(const QString& description)
{
    d->m_description = description;
}

void MSProcessor::setInputFileParameters(const QString& p1, const QString& p2, const QString& p3, const QString& p4)
{
    if (!p1.isEmpty())
        d->m_input_file_parameters << p1;
    if (!p2.isEmpty())
        d->m_input_file_parameters << p2;
    if (!p3.isEmpty())
        d->m_input_file_parameters << p3;
    if (!p4.isEmpty())
        d->m_input_file_parameters << p4;
}

void MSProcessor::setOutputFileParameters(const QString& p1, const QString& p2, const QString& p3, const QString& p4)
{
    if (!p1.isEmpty())
        d->m_output_file_parameters << p1;
    if (!p2.isEmpty())
        d->m_output_file_parameters << p2;
    if (!p3.isEmpty())
        d->m_output_file_parameters << p3;
    if (!p4.isEmpty())
        d->m_output_file_parameters << p4;
}

void MSProcessor::setRequiredParameters(const QString& p1, const QString& p2, const QString& p3, const QString& p4)
{
    if (!p1.isEmpty())
        d->m_required_parameters << p1;
    if (!p2.isEmpty())
        d->m_required_parameters << p2;
    if (!p3.isEmpty())
        d->m_required_parameters << p3;
    if (!p4.isEmpty())
        d->m_required_parameters << p4;
}

void MSProcessor::setOptionalParameters(const QString& p1, const QString& p2, const QString& p3, const QString& p4)
{
    if (!p1.isEmpty())
        d->m_optional_parameters << p1;
    if (!p2.isEmpty())
        d->m_optional_parameters << p2;
    if (!p3.isEmpty())
        d->m_optional_parameters << p3;
    if (!p4.isEmpty())
        d->m_optional_parameters << p4;
}

bool MSProcessor::checkParameters(const QMap<QString, QVariant>& params)
{
    QStringList required;
    required.append(d->m_input_file_parameters);
    required.append(d->m_output_file_parameters);
    required.append(d->m_required_parameters);
    QStringList optional;
    optional.append(d->m_optional_parameters);
    foreach (QString req, required) {
        if (!params.contains(req)) {
            qWarning() << QString("Processor %1 missing required parameter: %2").arg(this->name()).arg(req);
            return false;
        }
    }
    QStringList keys = params.keys();
    foreach (QString key, keys) {
        if ((!required.contains(key)) && (!optional.contains(key))) {
            qWarning() << QString("Processor %1: invalid parameter: %2").arg(this->name()).arg(key);
            return false;
        }
    }
    return true;
}

/*
The input is a test_number (ranging 0,1,2,...) and file_params. The framework will create paths to temporary files for each input and output file parameter. Recall that every processor operates on a collection of named input files and input parameters and produces a collection of named output files. The test is responsible for (a) writing test data to the temporary input files (b) setting test values of the parameters (c) running the test to create the temporary output files and (d) if possible checking the results, although in general this is not done.

The framework will then compute the checksums of the input files and the checksums of the output files, and create two hash codes: input_code that depends uniquely on the input parameter and input file checksums, and output_code that depends uniquely on the output file checksums.

The output of running
> mountainsort
could be like this...

...
...
PASSED copy 0 8790bb7aa3e207f5746f918f794f406ae1130919 db6f7b45de749f970890b5646dfe70150ff184d0
PASSED copy 1 a8c650302cbca9eaf8e64b20a7fac0fc82c302e6 401e98f51c2e0a96384a573e1f0c41fbac4de96f
PASSED normalize_channels 2 85d124e05e2c249b40a52d53578ce912d8ace221 8422014e62fdc893080599ee5379989e9c8f77f7

There should be a record kept of all the input-code->output-code pairs. If in a future test we have the same input-code corresponding to a different output-code then we raise a red flag indicating that something has changed. That's a part I have not yet implemented.
*/
