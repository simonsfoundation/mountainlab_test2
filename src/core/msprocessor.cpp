#include "msprocessor.h"

class MSProcessorPrivate
{
public:
	MSProcessor *q;
	QString m_name;
};

MSProcessor::MSProcessor() {
	d=new MSProcessorPrivate;
	d->q=this;
}

MSProcessor::~MSProcessor() {
	delete d;
}

QString MSProcessor::name()
{
	return d->m_name;
}

void MSProcessor::setName(const QString &name)
{
	d->m_name=name;
}

bool MSProcessor::checkParameters(const QMap<QString, QVariant> &params, const QStringList &required, const QStringList &optional)
{
	foreach (QString req,required) {
		if (!params.contains(req)) {
			qWarning() << QString("Processor %1 missing required parameter: %2").arg(this->name()).arg(req);
			return false;
		}
	}
	QStringList keys=params.keys();
	foreach (QString key,keys) {
		if ((!required.contains(key))&&(!optional.contains(key))) {
			qWarning() << QString("Processor %1: invalid parameter: %2").arg(this->name()).arg(key);
			return false;
		}
	}
	return true;
}

