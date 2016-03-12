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

#ifndef MSPROCESSORMANAGER_H
#define MSPROCESSORMANAGER_H

#include "msprocessor.h"

#include <QString>
#include <QMap>
#include <QVariant>

class MSProcessorManagerPrivate;
class MSProcessorManager {
public:
	friend class MSProcessorManagerPrivate;
	MSProcessorManager();
	virtual ~MSProcessorManager();
	void loadDefaultProcessors();
	bool containsProcessor(const QString &processor_name) const;
	bool checkProcessor(const QString &processor_name,const QMap<QString,QVariant> &parameters) const;
	bool runProcessor(const QString &processor_name,const QMap<QString,QVariant> &parameters) const;

	void loadProcessor(MSProcessor *P);
	QStringList allProcessorNames() const;

	QString usageString() const;
private:
	MSProcessorManagerPrivate *d;
};

#endif // MSPROCESSORMANAGER_H

