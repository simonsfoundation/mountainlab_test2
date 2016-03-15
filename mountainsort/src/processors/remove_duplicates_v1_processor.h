#ifndef REMOVE_DUPLICATES_V1_PROCESSOR_H
#define REMOVE_DUPLICATES_V1_PROCESSOR_H

#include "msprocessor.h"

class remove_duplicates_v1_ProcessorPrivate;
class remove_duplicates_v1_Processor : public MSProcessor
{
public:
	friend class remove_duplicates_v1_ProcessorPrivate;
	remove_duplicates_v1_Processor();
	virtual ~remove_duplicates_v1_Processor();

	bool check(const QMap<QString,QVariant> &params);
	bool run(const QMap<QString,QVariant> &params);
private:
	remove_duplicates_v1_ProcessorPrivate *d;
};

#endif // REMOVE_DUPLICATES_V1_PROCESSOR_H

