#ifndef COPY_PROCESSOR_H
#define COPY_PROCESSOR_H

#include "msprocessor.h"

class copy_ProcessorPrivate;
class copy_Processor : public MSProcessor
{
public:
	friend class copy_ProcessorPrivate;
	copy_Processor();
	virtual ~copy_Processor();

	bool check(const QMap<QString,QVariant> &params);
	bool run(const QMap<QString,QVariant> &params);
private:
	copy_ProcessorPrivate *d;
};

#endif // COPY_PROCESSOR_H

