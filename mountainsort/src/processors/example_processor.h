/******************************************************
**
** Copyright (C) 2016 by Jeremy Magland
**
** This file is part of the MountainSort C++ project
**
** Some rights reserved.
** See accompanying LICENSE and README files.
**
*******************************************************/

#ifndef EXAMPLE_PROCESSOR_H
#define EXAMPLE_PROCESSOR_H

#include "msprocessor.h"

class example_ProcessorPrivate;
class example_Processor : public MSProcessor
{
public:
	friend class example_ProcessorPrivate;
	example_Processor();
	virtual ~example_Processor();

	bool check(const QMap<QString,QVariant> &params);
	bool run(const QMap<QString,QVariant> &params);
private:
	example_ProcessorPrivate *d;
};

#endif // EXAMPLE_PROCESSOR_H

