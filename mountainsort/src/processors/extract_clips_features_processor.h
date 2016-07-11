/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/5/2016
*******************************************************/

#ifndef EXTRACT_CLIPS_FEATURES_PROCESSOR_H
#define EXTRACT_CLIPS_FEATURES_PROCESSOR_H

#include "msprocessor.h"

class extract_clips_features_ProcessorPrivate;
class extract_clips_features_Processor : public MSProcessor {
public:
    friend class extract_clips_features_ProcessorPrivate;
    extract_clips_features_Processor();
    virtual ~extract_clips_features_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    extract_clips_features_ProcessorPrivate* d;
};

#endif // EXTRACT_CLIPS_FEATURES_PROCESSOR_H
