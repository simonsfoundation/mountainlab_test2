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
	bool checkParameters(const QMap<QString,QVariant> &params,const QStringList &required,const QStringList &optional);
private:
	MSProcessorPrivate *d;
};

#endif // MSPROCESSOR_H

