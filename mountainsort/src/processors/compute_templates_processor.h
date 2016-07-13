/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/4/2016
*******************************************************/

#ifndef COMPUTE_TEMPLATES_PROCESSOR_H
#define COMPUTE_TEMPLATES_PROCESSOR_H

#include "msprocessor.h"

class compute_templates_ProcessorPrivate;
class compute_templates_Processor : public MSProcessor {
public:
    friend class compute_templates_ProcessorPrivate;
    compute_templates_Processor();
    virtual ~compute_templates_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    compute_templates_ProcessorPrivate* d;
};

#endif // COMPUTE_TEMPLATES_PROCESSOR_H
