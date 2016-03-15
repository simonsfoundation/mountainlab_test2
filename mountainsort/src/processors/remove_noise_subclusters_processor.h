#ifndef REMOVE_NOISE_SUBCLUSTERS_PROCESSOR_H
#define REMOVE_NOISE_SUBCLUSTERS_PROCESSOR_H

#include "msprocessor.h"

class remove_noise_subclusters_ProcessorPrivate;
class remove_noise_subclusters_Processor : public MSProcessor
{
public:
	friend class remove_noise_subclusters_ProcessorPrivate;
	remove_noise_subclusters_Processor();
	virtual ~remove_noise_subclusters_Processor();

	bool check(const QMap<QString,QVariant> &params);
	bool run(const QMap<QString,QVariant> &params);
private:
	remove_noise_subclusters_ProcessorPrivate *d;
};

#endif // REMOVE_NOISE_SUBCLUSTERS_PROCESSOR_H

