#include "example_processor.h"

class example_ProcessorPrivate {
public:
    example_Processor* q;
};

example_Processor::example_Processor()
{
    d = new example_ProcessorPrivate;
    d->q = this;

    this->setName("example");
    this->setRequiredParameters("param1");
    this->setOptionalParameters("param2");
}

example_Processor::~example_Processor()
{
    delete d;
}

bool example_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool example_Processor::run(const QMap<QString, QVariant>& params)
{
    QString param1 = params["param1"].toString();
    QString param2 = params["param2"].toString();
    printf("Running example processor.\n");
    printf("  Required param1=%s\n", param1.toLatin1().data());
    printf("  Optional param2=%s\n", param2.toLatin1().data());
    return true;
}
