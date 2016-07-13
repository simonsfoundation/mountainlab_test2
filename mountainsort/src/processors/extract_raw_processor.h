/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/20/2016
*******************************************************/

#ifndef EXTRACT_RAW_PROCESSOR_H
#define EXTRACT_RAW_PROCESSOR_H

#include "msprocessor.h"

class extract_raw_ProcessorPrivate;
class extract_raw_Processor : public MSProcessor {
public:
    friend class extract_raw_ProcessorPrivate;
    extract_raw_Processor();
    virtual ~extract_raw_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    extract_raw_ProcessorPrivate* d;
};

QVector<int> str_to_intlist(QString str);

#endif // EXTRACT_RAW_PROCESSOR_H
