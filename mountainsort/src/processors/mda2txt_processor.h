/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef MDA2TXT_PROCESSOR_H
#define MDA2TXT_PROCESSOR_H

#include "msprocessor.h"

class mda2txt_ProcessorPrivate;
class mda2txt_Processor : public MSProcessor {
public:
    friend class mda2txt_ProcessorPrivate;
    mda2txt_Processor();
    virtual ~mda2txt_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    mda2txt_ProcessorPrivate* d;
};

#endif // MDA2TXT_PROCESSOR_H
