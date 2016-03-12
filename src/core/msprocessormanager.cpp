#include "msprocessormanager.h"
#include "mountainsort_version.h"
#include "msprocessor.h"
#include <QList>
#include <QTime>

#include "example_processor.h"

class MSProcessorManagerPrivate
{
public:
	MSProcessorManager *q;
	QMap<QString,MSProcessor *> m_processors;
	example_Processor m_dummy_processor;

	MSProcessor *find_processor(const QString &name);
};

MSProcessorManager::MSProcessorManager() {
	d=new MSProcessorManagerPrivate;
	d->q=this;
}

MSProcessorManager::~MSProcessorManager() {
	qDeleteAll(d->m_processors);
	delete d;
}

void MSProcessorManager::loadDefaultProcessors()
{
	loadProcessor(new example_Processor);
}

bool MSProcessorManager::containsProcessor(const QString &processor_name) const
{
	return d->m_processors.contains(processor_name);
}

bool MSProcessorManager::checkProcessor(const QString &processor_name, const QMap<QString, QVariant> &parameters) const
{
	return d->find_processor(processor_name)->check(parameters);
}

bool MSProcessorManager::runProcessor(const QString &processor_name, const QMap<QString, QVariant> &parameters) const
{
	printf("RUNNING %s",processor_name.toLatin1().data());
	QTime timer; timer.start();
	bool ret=d->find_processor(processor_name)->run(parameters);
	printf("Elapsed time for processor %s: %g sec\n",processor_name.toLatin1().data(),timer.elapsed()*1.0/1000);

	return ret;
}

void MSProcessorManager::loadProcessor(MSProcessor *P)
{
	if (d->m_processors.contains(P->name())) {
		qWarning() << "Processor with this name has already been loaded:" << P->name();
		return;
	}
	d->m_processors[P->name()]=P;
}

QStringList MSProcessorManager::allProcessorNames() const
{
	return d->m_processors.keys();
}

QString MSProcessorManager::usageString() const
{
	QString str;
	str+=QString("MountainSort version %1\n").arg(mountainsort_version());

	str+=QString("\nUsage:\n");
	str+=QString("mountainsort processor_name --param1=val1 --param2=val2\n");

	str+=QString("\nAvailable processors:\n");
	QStringList names=allProcessorNames();
	foreach (QString name,names) {
		str+=name+" ";
	}
	str+="\n";

	return str;
}

MSProcessor *MSProcessorManagerPrivate::find_processor(const QString &name)
{
	if (!m_processors.contains(name)) {
		qWarning() << "Unable to find processor: " << name;
		return &m_dummy_processor;
	}
	return m_processors[name];
}
