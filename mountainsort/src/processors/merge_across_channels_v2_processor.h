/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/22/2016
*******************************************************/

#ifndef MERGE_ACROSS_CHANNELS_V2_PROCESSOR_H
#define MERGE_ACROSS_CHANNELS_V2_PROCESSOR_H

#include "msprocessor.h"

class merge_across_channels_v2_ProcessorPrivate;
class merge_across_channels_v2_Processor : public MSProcessor {
public:
    friend class merge_across_channels_v2_ProcessorPrivate;
    merge_across_channels_v2_Processor();
    virtual ~merge_across_channels_v2_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    merge_across_channels_v2_ProcessorPrivate* d;
};

#endif // MERGE_ACROSS_CHANNELS_V2_PROCESSOR_H
