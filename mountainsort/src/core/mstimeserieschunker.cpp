#include "mstimeserieschunker.h"
#include "diskreadmda.h"

class MSTimeSeriesChunkerPrivate
{
public:
	MSTimeSeriesChunker *q;
	DiskReadMda m_array;
	long m_current_timepoint;
};

MSTimeSeriesChunker::MSTimeSeriesChunker(const QString &path)
{
	d=new MSTimeSeriesChunkerPrivate;
	d->q=this;
	d->m_current_timepoint=0;
	d->m_array.setPath(path);
}

MSTimeSeriesChunker::~MSTimeSeriesChunker() {
	delete d;
}

long MSTimeSeriesChunker::N1()
{
	return d->m_array.N1();
}

long MSTimeSeriesChunker::N2()
{
	return d->m_array.N2();
}

void MSTimeSeriesChunker::rewind(long num_timepoints)
{
	d->m_current_timepoint-=num_timepoints;
}

bool MSTimeSeriesChunker::loadNextChunk(Mda &ret,long num_timepoints)
{
	if (d->m_current_timepoint>=this->N2()) return false;
	d->m_array.getSubArray(ret,0,d->m_current_timepoint,this->N1(),num_timepoints);
	d->m_current_timepoint+=num_timepoints;
	return true;
}

