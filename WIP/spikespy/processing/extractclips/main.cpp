#include <QCoreApplication>
#include <QFileInfo>
#include "commandlineparams.h"
#include "extractclips.h"
#include <QDebug>

void usage()
{
    printf("extractclips inpath inpath_TL outpath --clipsize=[clipsize] --labels=1,2,3\n");
    printf("  You can also use --labels=all\n");
    printf("Optional parameters: --fixed-clipsize\n");
}

int main(int argc, char* argv[])
{
    CLParams params = commandlineparams(argc, argv);

    if (!params.success) {
        qCritical() << params.error_message;
        usage();
        return -1;
    }
    if (params.unnamed_parameters.count() != 3) {
        usage();
        return -1;
    }

    QString inpath = params.unnamed_parameters[0];
    QString inpath_TL = params.unnamed_parameters[1];
    QString outpath = params.unnamed_parameters[2];

    QString outpath_TL = QFileInfo(outpath).path() + "/" + QFileInfo(outpath).completeBaseName() + ".TL.mda";
    QString outpath_TM = QFileInfo(outpath).path() + "/" + QFileInfo(outpath).completeBaseName() + ".TM.mda";

    QMap<QString, QVariant> runparams;

    int clipsize = params.named_parameters["clipsize"].toInt();
    if ((clipsize <= 2) || (clipsize >= 1000)) {
        qCritical() << "invalid clipsize" << clipsize;
        usage();
        return -1;
    }
    runparams["clipsize"] = clipsize;
    if (params.named_parameters.contains("fixed-clipsize"))
        runparams["fixed-clipsize"] = true;

    /*QStringList labels=params.named_parameters["labels"].split(",");
	if (labels.count()==0) {
		qCritical() << "invalid labels" << params.named_parameters["labels"];
		usage(); return -1;
	}
	runparams["labels"]=labels;*/

    runparams["labels"] = params.named_parameters["labels"];

    FILE* inf = fopen(inpath.toLatin1().data(), "rb");
    if (!inf) {
        printf("Unable to open input file for reading.\n");
        return -1;
    }
    FILE* inf_TL = fopen(inpath_TL.toLatin1().data(), "rb");
    if (!inf_TL) {
        printf("Unable to open input_TL file for reading.\n");
        fclose(inf);
        return -1;
    }
    FILE* outf = fopen(outpath.toLatin1().data(), "wb");
    if (!outf) {
        printf("Unable to open output file for writing.\n");
        fclose(inf);
        fclose(inf_TL);
        return -1;
    }
    FILE* outf_TL = fopen(outpath_TL.toLatin1().data(), "wb");
    if (!outf_TL) {
        printf("Unable to open output_TL file for writing.\n");
        fclose(inf);
        fclose(inf_TL);
        fclose(outf);
        return -1;
    }
    FILE* outf_TM = fopen(outpath_TM.toLatin1().data(), "wb");
    if (!outf_TM) {
        printf("Unable to open output_TM file for writing.\n");
        fclose(inf);
        fclose(inf_TL);
        fclose(outf);
        fclose(outf_TL);
        return -1;
    }

    qDebug() << "Running extractclips" << inpath << inpath_TL << outpath << outpath_TL << outpath_TM << runparams["clipsize"].toInt();
    if (!extractclips(inf, inf_TL, outf, outf_TL, outf_TM, runparams)) {
        qCritical() << "Problem in extractclips.";
    }

    fclose(inf);
    fclose(outf);
    fclose(outf_TL);
    fclose(outf_TM);

    return 0;
}
