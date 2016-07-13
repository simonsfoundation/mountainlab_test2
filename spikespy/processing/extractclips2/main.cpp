#include <QCoreApplication>
#include <QFileInfo>
#include "commandlineparams.h"
#include "extractclips2.h"
#include <QDebug>

void usage()
{
    printf("extractclips inpath1 inpath2 inpath_TL1 inpath_TL2 outpath1 outpath2 --clipsize=[clipsize] --labels=1,2\n");
    printf("Optional parameters:\n");
}

int main(int argc, char* argv[])
{
    CLParams params = commandlineparams(argc, argv);

    if (!params.success) {
        qCritical() << params.error_message;
        usage();
        return -1;
    }
    if (params.unnamed_parameters.count() != 6) {
        usage();
        return -1;
    }

    QString inpath1 = params.unnamed_parameters[0];
    QString inpath2 = params.unnamed_parameters[1];
    QString inpath_TL1 = params.unnamed_parameters[2];
    QString inpath_TL2 = params.unnamed_parameters[3];
    QString outpath1 = params.unnamed_parameters[4];
    QString outpath2 = params.unnamed_parameters[5];

    QString outpath_TL = QFileInfo(outpath1).path() + "/" + QFileInfo(outpath1).completeBaseName() + ".TL.mda";
    QString outpath_TM = QFileInfo(outpath1).path() + "/" + QFileInfo(outpath1).completeBaseName() + ".TM.mda";

    QMap<QString, QVariant> runparams;

    int clipsize = params.named_parameters["clipsize"].toInt();
    if ((clipsize <= 2) || (clipsize >= 1000)) {
        qCritical() << "invalid clipsize" << clipsize;
        usage();
        return -1;
    }
    runparams["clipsize"] = clipsize;

    QStringList labels = params.named_parameters["labels"].toString().split(",");
    if (labels.count() != 2) {
        qCritical() << "invalid labels" << params.named_parameters["labels"];
        usage();
        return -1;
    }
    runparams["labels"] = labels;

    FILE* inf1 = fopen(inpath1.toLatin1().data(), "rb");
    if (!inf1) {
        printf("Unable to open input file1 for reading.\n");
        return -1;
    }
    FILE* inf2 = fopen(inpath2.toLatin1().data(), "rb");
    if (!inf2) {
        printf("Unable to open input file2 for reading.\n");
        return -1;
    }
    FILE* inf_TL1 = fopen(inpath_TL1.toLatin1().data(), "rb");
    if (!inf_TL1) {
        printf("Unable to open input_TL1 file for reading.\n");
        return -1;
    }
    FILE* inf_TL2 = fopen(inpath_TL2.toLatin1().data(), "rb");
    if (!inf_TL2) {
        printf("Unable to open input_TL2 file for reading.\n");
        return -1;
    }
    FILE* outf1 = fopen(outpath1.toLatin1().data(), "wb");
    if (!outf1) {
        printf("Unable to open output file1 for writing.\n");
        return -1;
    }
    FILE* outf2 = fopen(outpath2.toLatin1().data(), "wb");
    if (!outf2) {
        printf("Unable to open output file2 for writing.\n");
        return -1;
    }
    FILE* outf_TL = fopen(outpath_TL.toLatin1().data(), "wb");
    if (!outf_TL) {
        printf("Unable to open output_TL file for writing.\n");
        return -1;
    }
    FILE* outf_TM = fopen(outpath_TM.toLatin1().data(), "wb");
    if (!outf_TM) {
        printf("Unable to open output_TM file for writing.\n");
        return -1;
    }

    qDebug() << "Running extractclips2" << inpath1 << inpath2 << inpath_TL1 << inpath_TL2 << outpath1 << outpath2 << outpath_TM << runparams["clipsize"].toInt();
    if (!extractclips2(inf1, inf2, inf_TL1, inf_TL2, outf1, outf2, outf_TL, outf_TM, runparams)) {
        qCritical() << "Problem in extractclips.";
    }

    fclose(inf1);
    fclose(inf2);
    fclose(inf_TL1);
    fclose(inf_TL2);
    fclose(outf1);
    fclose(outf2);
    fclose(outf_TL);
    fclose(outf_TM);

    return 0;
}
