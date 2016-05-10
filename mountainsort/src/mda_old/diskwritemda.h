/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef DISKWRITEMDA_H
#define DISKWRITEMDA_H

#include "mda.h"

#include <QString>
#include <mdaio.h>

class DiskWriteMdaPrivate;
class DiskWriteMda
{
public:
	friend class DiskWriteMdaPrivate;
	DiskWriteMda();
	DiskWriteMda(int data_type,const QString &path,long N1,long N2,long N3=1,long N4=1,long N5=1,long N6=1);
	virtual ~DiskWriteMda();
	bool open(int data_type,const QString &path,long N1,long N2,long N3=1,long N4=1,long N5=1,long N6=1);
	void close();

	long N1();
	long N2();
	long N3();
	long N4();
	long N5();
	long N6();
	long totalSize();

    void writeChunk(Mda &X,long i);
    void writeChunk(Mda &X,long i1,long i2);
    void writeChunk(Mda &X,long i1,long i2,long i3);
private:
	DiskWriteMdaPrivate *d;
};

#endif // DISKWRITEMDA_H
