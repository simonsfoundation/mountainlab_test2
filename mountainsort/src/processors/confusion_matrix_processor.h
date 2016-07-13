/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/19/2016
*******************************************************/

#ifndef CONFUSION_MATRIX_PROCESSOR_H
#define CONFUSION_MATRIX_PROCESSOR_H

#include "msprocessor.h"

class confusion_matrix_ProcessorPrivate;
class confusion_matrix_Processor : public MSProcessor {
public:
    friend class confusion_matrix_ProcessorPrivate;
    confusion_matrix_Processor();
    virtual ~confusion_matrix_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    confusion_matrix_ProcessorPrivate* d;
};

#endif // CONFUSION_MATRIX_PROCESSOR_H
