#include "msprocessmanager.h"
#include "mountainsort_version.h"
#include "msprocessor.h"
#include <QList>
#include <QTime>
#include <QCoreApplication>
#include <QDir>
#include <QCryptographicHash>

#include "example_processor.h"
#include "bandpass_filter_processor.h"
#include "whiten_processor.h"
#include "detect_processor.h"
#include "branch_cluster_v2_processor.h"
#include "remove_duplicate_clusters_processor.h"
#include "remove_noise_subclusters_processor.h"
#include "compute_outlier_scores_processor.h"
#include "copy_processor.h"
#include "mda2txt_processor.h"
#include "mask_out_artifacts_processor.h"

#include "qjson.h"
#include "textfile.h"
#include <sys/stat.h>

class MSProcessManagerPrivate
{
public:
	MSProcessManager *q;
	QMap<QString,MSProcessor *> m_processors;
	example_Processor m_dummy_processor;

	MSProcessor *find_processor(const QString &name);
	QString process_directory();
	QMap<QString,QVariant> compute_process_info(const QString &processor_name,const QMap<QString,QVariant> &parameters);
	void write_process_record(const QString &processor_name,const QMap<QString,QVariant> &parameters);
};

MSProcessManager::MSProcessManager() {
	d=new MSProcessManagerPrivate;
	d->q=this;
}

MSProcessManager::~MSProcessManager() {
	qDeleteAll(d->m_processors);
	delete d;
}

void MSProcessManager::loadDefaultProcessors()
{
	loadProcessor(new example_Processor);
	loadProcessor(new bandpass_filter_Processor);
	loadProcessor(new whiten_Processor);
	loadProcessor(new detect_Processor);
	loadProcessor(new branch_cluster_v2_Processor);
    loadProcessor(new remove_duplicate_clusters_Processor);
	loadProcessor(new remove_noise_subclusters_Processor);
	loadProcessor(new compute_outlier_scores_Processor);
	loadProcessor(new copy_Processor);
	loadProcessor(new mda2txt_Processor);
    loadProcessor(new mask_out_artifacts_Processor);
}

bool MSProcessManager::containsProcessor(const QString &processor_name) const
{
	return d->m_processors.contains(processor_name);
}

bool MSProcessManager::checkProcess(const QString &processor_name, const QMap<QString, QVariant> &parameters) const
{
	return d->find_processor(processor_name)->check(parameters);
}

QString compute_hash(const QString &str) {
	QCryptographicHash hash(QCryptographicHash::Sha1);
	hash.addData(str.toLatin1());
	return QString(hash.result().toHex());
}

QString compute_process_code(const QString &processor_name,const QMap<QString,QVariant> &parameters) {
	QMap<QString,QVariant> X;
	X["processor_name"]=processor_name;
	X["parameters"]=parameters;
	QString json=toJSON(X);
	return compute_hash(json);
}

QString compute_file_code(const QString &path) {
	//the code comprises the device,inode,size, and modification time (in seconds)
	//note that it is not dependent on the file name
	struct stat SS;
	stat(path.toLatin1().data(),&SS);
	QString id_string=QString("%1:%2:%3:%4").arg(SS.st_dev).arg(SS.st_ino).arg(SS.st_size).arg(SS.st_mtim.tv_sec);
	return id_string;
}

bool MSProcessManager::findCompletedProcess(const QString &processor_name, const QMap<QString, QVariant> &parameters) const
{
	QString path=d->process_directory();
	QString code=compute_process_code(processor_name,parameters); //this code just depends on processor name and parameters
	QString fname=path+"/"+code+".process";
	if (!QFile::exists(fname)) return false; //file doesn't exist
	QMap<QString,QVariant> info=d->compute_process_info(processor_name,parameters); //this depends on processor name, version, parameters, and input/output file codes
	QString txt=read_text_file(fname);
	QMap<QString,QVariant> info_from_file=parseJSON(txt).toMap();
	return (toJSON(info)==toJSON(info_from_file)); //note: toJSON should be replaced by a canonical version (ie, one that produces a canonical test string, for example by alphabetizing the fields)
}

QMap<QString,QVariant> MSProcessManagerPrivate::compute_process_info(const QString &processor_name,const QMap<QString,QVariant> &parameters) {
	QMap<QString,QVariant> ret;
	QString version;
	QStringList input_file_parameters;
	QStringList output_file_parameters;
	if (m_processors.contains(processor_name)) {
		MSProcessor *PP=m_processors[processor_name];
		version=PP->version();
		input_file_parameters=PP->inputFileParameters();
		output_file_parameters=PP->outputFileParameters();
	}
	else return ret; //can't even find the processor (not registered)

	QMap<QString,QVariant> input_file_codes;
	foreach (QString pp,input_file_parameters) {
		QString path0=parameters[pp].toString();
        if (!path0.isEmpty()) {
            QString code0=compute_file_code(path0);
            input_file_codes[path0]=code0;
        }
	}
	QMap<QString,QVariant> output_file_codes;
	foreach (QString pp,output_file_parameters) {
		QString path0=parameters[pp].toString();
        if (!path0.isEmpty()) {
            QString code0=compute_file_code(path0);
            output_file_codes[path0]=code0;
        }
	}
	ret["processor_name"]=processor_name;
	ret["version"]=version;
	ret["parameters"]=parameters;
	ret["input_file_codes"]=input_file_codes;
	ret["output_file_codes"]=output_file_codes;
	return ret;
}

void MSProcessManagerPrivate::write_process_record(const QString &processor_name, const QMap<QString, QVariant> &parameters)
{
	QString path=process_directory();
	QString code=compute_process_code(processor_name,parameters);
	QString fname=path+"/"+code+".process";
	QMap<QString,QVariant> info=compute_process_info(processor_name,parameters);
	write_text_file(fname,toJSON(info));
}

bool MSProcessManager::runProcess(const QString &processor_name, const QMap<QString, QVariant> &parameters)
{
	printf("RUNNING %s\n",processor_name.toLatin1().data());
	QTime timer; timer.start();
	bool ret=d->find_processor(processor_name)->run(parameters);
	if (ret) {
		printf("Elapsed time for processor %s: %g sec\n",processor_name.toLatin1().data(),timer.elapsed()*1.0/1000);
		d->write_process_record(processor_name,parameters);
	}

	return ret;
}

void MSProcessManager::loadProcessor(MSProcessor *P)
{
	if (d->m_processors.contains(P->name())) {
		qWarning() << "Processor with this name has already been loaded:" << P->name();
		return;
	}
	d->m_processors[P->name()]=P;
}

QStringList MSProcessManager::allProcessorNames() const
{
	return d->m_processors.keys();
}

QString MSProcessManager::usageString() const
{
	QString str;
	str+=QString("MountainSort version %1\n").arg(mountainsort_version());

	str+=QString("\nUsage:\n");
	str+=QString("mountainsort processor_name --param1=val1 --param2=val2\n");

	str+=QString("\nAvailable processors:\n");
	QStringList names=allProcessorNames();
	for (int i=0; i<names.count(); i++) {
		QString name=names[i];
		str+=name;
		if (i+1<names.count()) str+=", ";
	}
	str+="\n";

    return str;
}

QString tostr(const QStringList &list) {
    QString ret;
    foreach (QString str,list) {
        ret+=str+" ";
    }
    return ret;
}

void MSProcessManager::printDetails() const
{
    QStringList names=allProcessorNames();
    for (int i=0; i<names.count(); i++) {
        QString name=names[i];
        MSProcessor *P=d->m_processors[name];
        QString str;
        str+=QString("***** Processor %1 *****").arg(P->name())+"\n";
        str+=QString("    Input files: %1").arg(tostr(P->inputFileParameters()))+"\n";
        str+=QString("    Output files: %1").arg(tostr(P->outputFileParameters()))+"\n";
        str+=QString("    Required params: %1").arg(tostr(P->requiredParameters()))+"\n";
        str+=QString("    Optional params: %1").arg(tostr(P->optionalParameters()))+"\n";
        printf("%s\n",str.toLatin1().data());
    }
}

MSProcessor *MSProcessManagerPrivate::find_processor(const QString &name)
{
	if (!m_processors.contains(name)) {
		qWarning() << "Unable to find processor: " << name;
		return &m_dummy_processor;
	}
	return m_processors[name];
}

QString MSProcessManagerPrivate::process_directory()
{
	QString path0=qApp->applicationDirPath();
	if (!QDir(path0).exists(".process_tracker")) {
		QDir(path0).mkdir(".process_tracker");
	}
	return path0+"/.process_tracker";
}
