#include <QCoreApplication>
#include "get_command_line_params.h"
//extern "C" {
#include "expfilter.h"
//}

void usage() {
	printf("expfilter inpath outpath --tau=[tau] --ftype=[lowpass|highpass]\n");
}

int main(int argc, char *argv[])
{
	QStringList required_params,optional_params;
	required_params << "tau" << "ftype";
	CLParams params=get_command_line_params(argc,argv,required_params,optional_params);

	if (!params.success) {
		qCritical() << params.error_message;
		usage(); return -1;
	}
	if (params.unnamed_parameters.count()!=2) {
		usage(); return -1;
	}

	QString inpath=params.unnamed_parameters[0];
	QString outpath=params.unnamed_parameters[1];

	int lowpass;
	if (params.named_parameters["ftype"]=="lowpass") lowpass=1;
	else if (params.named_parameters["ftype"]=="highpass") lowpass=0;
	else {usage(); return -1;}

	float tau=params.named_parameters["tau"].toFloat();

	FILE *inf=fopen(inpath.toLatin1().data(),"rb");
	if (!inf) {printf("Unable to open input file for reading.\n"); return -1;}
	FILE *outf=fopen(outpath.toLatin1().data(),"wb");
	if (!outf) {printf("Unable to open output file for writing.\n"); fclose(inf); return -1;}

	qDebug()  << "Running expfilter" << inpath << outpath << lowpass << tau;
	if (!expfilter(inf,outf,lowpass,tau)) {
		qCritical() << "Problem in expfilter.";
	}

	fclose(inf);
	fclose(outf);

	return 0;
}
