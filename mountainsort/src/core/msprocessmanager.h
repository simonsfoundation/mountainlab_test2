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

#ifndef MSPROCESSMANAGER_H
#define MSPROCESSMANAGER_H

#include "msprocessor.h"

#include <QString>
#include <QMap>
#include <QVariant>

class MSProcessManagerPrivate;
class MSProcessManager {
public:
	friend class MSProcessManagerPrivate;
	MSProcessManager();
	virtual ~MSProcessManager();
	void loadDefaultProcessors();
	bool containsProcessor(const QString &processor_name) const;
	bool checkProcess(const QString &processor_name,const QMap<QString,QVariant> &parameters) const;
	bool runProcess(const QString &processor_name,const QMap<QString,QVariant> &parameters);

	bool findCompletedProcess(const QString &processor_name,const QMap<QString,QVariant> &parameters) const;

	void loadProcessor(MSProcessor *P);
	QStringList allProcessorNames() const;

	QString usageString() const;
    void printDetails() const;
private:
	MSProcessManagerPrivate *d;
};

#endif // MSPROCESSMANAGER_H

