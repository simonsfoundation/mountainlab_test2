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

#ifndef MSTIMESERIESCHUNKER_H
#define MSTIMESERIESCHUNKER_H

#include <QString>
#include <mda.h>


class MSTimeSeriesChunkerPrivate;
class MSTimeSeriesChunker
{
public:
	friend class MSTimeSeriesChunkerPrivate;
	MSTimeSeriesChunker(const QString &path);
	virtual ~MSTimeSeriesChunker();
	long N1();
	long N2();
	void rewind(long num_timepoints);
	bool loadNextChunk(Mda &ret,long num_timepoints);
private:
	MSTimeSeriesChunkerPrivate *d;
};

#endif // MSTIMESERIESCHUNKER_H

