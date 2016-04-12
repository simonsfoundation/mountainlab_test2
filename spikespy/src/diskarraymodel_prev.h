#ifndef DISKARRAYMODEL_H
#define DISKARRAYMODEL_H

#include <QObject>
#include "mda.h"

//changed all ints to longs!!! 3/10/2016 (hope it didn't cause problems!)

class DiskArrayModelPrivate;
class DiskArrayModel : public QObject
{
	Q_OBJECT
public:
	friend class DiskArrayModelPrivate;
	explicit DiskArrayModel(QObject *parent = 0);
	~DiskArrayModel();
	void setPath(QString path);
	void setFromMda(const Mda &X);
	QString path();
	bool fileHierarchyExists();
	void createFileHierarchyIfNeeded();
    double value(long ch,long t);

    virtual Mda loadData(long scale,long t1,long t2);
    virtual long size(long dim);
    virtual long dim3();
private:
	DiskArrayModelPrivate *d;

signals:

public slots:
};

#endif // DISKARRAYMODEL_H
