/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/4/2016
*******************************************************/

#ifndef MV_SUBFIRINGS_H
#define MV_SUBFIRINGS_H

#include "msprocessor.h"

class mv_subfirings_ProcessorPrivate;
class mv_subfirings_Processor : public MSProcessor {
public:
    friend class mv_subfirings_ProcessorPrivate;
    mv_subfirings_Processor();
    virtual ~mv_subfirings_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    mv_subfirings_ProcessorPrivate* d;
};

#endif // MV_SUBFIRINGS_H
