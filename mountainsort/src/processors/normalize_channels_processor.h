/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef NORMALIZE_CHANNELS_PROCESSOR_H
#define NORMALIZE_CHANNELS_PROCESSOR_H

#include "msprocessor.h"

class normalize_channels_ProcessorPrivate;
class normalize_channels_Processor : public MSProcessor {
public:
    friend class normalize_channels_ProcessorPrivate;
    normalize_channels_Processor();
    virtual ~normalize_channels_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);
    MSProcessorTestResults runTest(int test_number, const QMap<QString, QVariant>& file_params) Q_DECL_OVERRIDE;

private:
    normalize_channels_ProcessorPrivate* d;
};

#endif // NORMALIZE_CHANNELS_PROCESSOR_H
