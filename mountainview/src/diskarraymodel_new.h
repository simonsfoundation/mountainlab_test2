#ifndef DISKARRAYMODEL_NEW_H
#define DISKARRAYMODEL_NEW_H

#include <QString>
#include "mda.h"

class DiskArrayModel_NewPrivate;
class DiskArrayModel_New
{
public:
	friend class DiskArrayModel_NewPrivate;
	DiskArrayModel_New();
	~DiskArrayModel_New();
	void setPath(QString path);
	void setFromMda(const Mda &X);
	QString path();
	bool fileHierarchyExists();
	void createFileHierarchyIfNeeded();
    double value(long ch,long t);
    virtual Mda loadData(long scale,long t1,long t2);
	long numChannels() const; //size of first dimension
	long numTimepoints() const; //product of 2nd and 3rd dimensions!
	long clipSize() const; //2nd dimension
private:
	DiskArrayModel_NewPrivate *d;
};

#endif // DISKARRAYMODEL_NEW_H
