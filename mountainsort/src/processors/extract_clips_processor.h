/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/4/2016
*******************************************************/

#ifndef EXTRACT_CLIPS_PROCESSOR_H
#define EXTRACT_CLIPS_PROCESSOR_H

#include "msprocessor.h"

class extract_clips_ProcessorPrivate;
class extract_clips_Processor : public MSProcessor {
public:
    friend class extract_clips_ProcessorPrivate;
    extract_clips_Processor();
    virtual ~extract_clips_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    extract_clips_ProcessorPrivate* d;
};

#endif // EXTRACT_CLIPS_PROCESSOR_H
