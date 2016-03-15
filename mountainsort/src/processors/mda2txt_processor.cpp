#include "mda2txt_processor.h"
#include "diskreadmda.h"
#include "textfile.h"

class mda2txt_ProcessorPrivate
{
public:
	mda2txt_Processor *q;
};

mda2txt_Processor::mda2txt_Processor() {
	d=new mda2txt_ProcessorPrivate;
	d->q=this;

	this->setName("mda2txt");
	this->setVersion("0.1");
	this->setInputFileParameters("mda_file");
	this->setOutputFileParameters("txt_file");
	this->setOptionalParameters("transpose","max_rows","max_cols","delimiter");
}

mda2txt_Processor::~mda2txt_Processor() {
	delete d;
}

bool mda2txt_Processor::check(const QMap<QString, QVariant> &params)
{
	if (!this->checkParameters(params)) return false;
	return true;
}

QString format_number(double num) {
	if (num==(long)num) {
		return QString::number(num,'f',0);
	}
	else {
		return QString::number(num,'f');
	}
}

bool mda2txt_Processor::run(const QMap<QString, QVariant> &params)
{
	QString mda_path=params["mda_file"].toString();
	QString txt_path=params["txt_file"].toString();
	int transpose=params.value("transpose",1).toInt();
	long max_rows=params.value("max_rows",1e9).toLongLong();
	long max_cols=params.value("max_cols",200).toLongLong();
	QString delimiter=params.value("delimeter","tab").toString();

	if (delimiter=="tab") delimiter="\t";
	if (delimiter=="comma") delimiter=",";

	DiskReadMda X; X.setPath(mda_path);
	QString txt;
	if (transpose) {
		if (X.N1()>max_cols) {
			printf("Too many columns::: %ld>%ld\n",(long)X.N1(),max_cols);
			return false;
		}
		if (X.N2()>max_rows) {
			printf("Too many rows::: %ld>%ld\n",(long)X.N2(),max_rows);
			return false;
		}
		for (int i=0; i<X.N2(); i++) {
			QString line;
			for (int j=0; j<X.N1(); j++) {
				line+=QString("%1").arg(format_number(X.value(j,i)));
				if (j+1<X.N1()) line+=QString("%1").arg(delimiter);
			}
			txt+=line+"\n";
		}
	}
	else {
		if (X.N1()>max_rows) {
			printf("Too many rows: %ld\n",(long)X.N1());
			return false;
		}
		if (X.N2()>max_cols) {
			printf("Too many columns: %ld\n",(long)X.N2());
			return false;
		}
		for (int i=0; i<X.N1(); i++) {
			QString line;
			for (int j=0; j<X.N2(); j++) {
				line+=QString("%1").arg(format_number(X.value(i,j)));
				if (j+1<X.N1()) line+=QString("%1").arg(delimiter);
			}
			txt+=line+"\n";
		}
	}
	printf("Writing %d bytes to %s...\n",txt.count(),txt_path.toLatin1().data());
	if (!write_text_file(txt_path,txt)) {
		printf("Unable to write file %s\n",txt_path.toLatin1().data());
		return false;
	}
	return true;
}


