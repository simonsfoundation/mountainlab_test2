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

#ifndef MSPROCESSOR_H
#define MSPROCESSOR_H

#include <QString>
#include <QVariant>
#include <QDebug>

class MSProcessorPrivate;
class MSProcessor
{
public:
	friend class MSProcessorPrivate;
	MSProcessor();
	virtual ~MSProcessor();

	QString name();

	virtual bool check(const QMap<QString,QVariant> &params)=0;
	virtual bool run(const QMap<QString,QVariant> &params)=0;

protected:
	void setName(const QString &name);
	void setInputFileParameters(const QString &p1,const QString &p2="",const QString &p3="",const QString &p4="");
	void setOutputFileParameters(const QString &p1,const QString &p2="",const QString &p3="",const QString &p4="");
	bool checkParameters(const QMap<QString,QVariant> &params,const QStringList &required,const QStringList &optional);
private:
	MSProcessorPrivate *d;
};

#endif // MSPROCESSOR_H

