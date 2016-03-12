#include "msprocessor.h"

class MSProcessorPrivate
{
public:
	MSProcessor *q;
	QString m_name;
	QStringList m_input_file_parameters;
	QStringList m_output_file_parameters;
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

void MSProcessor::setInputFileParameters(const QString &p1, const QString &p2, const QString &p3, const QString &p4)
{
	if (!p1.isEmpty()) d->m_input_file_parameters << p1;
	if (!p2.isEmpty()) d->m_input_file_parameters << p2;
	if (!p3.isEmpty()) d->m_input_file_parameters << p3;
	if (!p4.isEmpty()) d->m_input_file_parameters << p4;
}

void MSProcessor::setOutputFileParameters(const QString &p1, const QString &p2, const QString &p3, const QString &p4)
{
	if (!p1.isEmpty()) d->m_output_file_parameters << p1;
	if (!p2.isEmpty()) d->m_output_file_parameters << p2;
	if (!p3.isEmpty()) d->m_output_file_parameters << p3;
	if (!p4.isEmpty()) d->m_output_file_parameters << p4;
}

bool MSProcessor::checkParameters(const QMap<QString, QVariant> &params, const QStringList &required, const QStringList &optional)
{
	QStringList required2=required;
	required2.append(d->m_input_file_parameters);
	required2.append(d->m_output_file_parameters);
	foreach (QString req,required2) {
		if (!params.contains(req)) {
			qWarning() << QString("Processor %1 missing required parameter: %2").arg(this->name()).arg(req);
			return false;
		}
	}
	QStringList keys=params.keys();
	foreach (QString key,keys) {
		if ((!required2.contains(key))&&(!optional.contains(key))) {
			qWarning() << QString("Processor %1: invalid parameter: %2").arg(this->name()).arg(key);
			return false;
		}
	}
	return true;
}

