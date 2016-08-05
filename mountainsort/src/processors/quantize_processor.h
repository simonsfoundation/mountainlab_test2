/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef QUANTIZE_PROCESSOR_H
#define QUANTIZE_PROCESSOR_H

#include "msprocessor.h"

class quantize_ProcessorPrivate;
class quantize_Processor : public MSProcessor {
public:
    friend class quantize_ProcessorPrivate;
    quantize_Processor();
    virtual ~quantize_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);
    MSProcessorTestResults runTest(int test_number, const QMap<QString, QVariant>& file_params) Q_DECL_OVERRIDE;

private:
    quantize_ProcessorPrivate* d;
};

#endif // COPY_PROCESSOR_H
