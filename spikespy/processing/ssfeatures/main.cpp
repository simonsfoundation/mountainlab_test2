#include <QCoreApplication>
#include <QFileInfo>
#include "commandlineparams.h"
#include "ssfeatures.h"
#include <QDebug>

void usage()
{
    printf("ssfeatures inpath outpath --method=pca --nfeatures=[#features]\n");
    printf("Optional parameters: --niterations=[#iterations]\n");
}

int main(int argc, char* argv[])
{
    CLParams params = commandlineparams(argc, argv);

    if (!params.success) {
        qCritical() << params.error_message;
        usage();
        return -1;
    }
    if (params.unnamed_parameters.count() != 2) {
        usage();
        return -1;
    }
    if (params.named_parameters["method"] != "pca") {
        usage();
        return -1;
    }

    QString inpath = params.unnamed_parameters[0];
    QString outpath = params.unnamed_parameters[1];

    QMap<QString, QVariant> runparams;

    int nfeatures = params.named_parameters["nfeatures"].toInt();
    if ((nfeatures < 1) || (nfeatures >= 500)) {
        qCritical() << "invalid nfeatures" << nfeatures;
        usage();
        return -1;
    }
    runparams["nfeatures"] = nfeatures;

    int niterations = params.named_parameters.value("niterations", "10").toInt();
    if ((niterations < 1) || (niterations >= 500)) {
        qCritical() << "invalid niterations" << niterations;
        usage();
        return -1;
    }
    runparams["niterations"] = niterations;

    FILE* inf = fopen(inpath.toLatin1().data(), "rb");
    if (!inf) {
        printf("Unable to open input file for reading.\n");
        return -1;
    }
    FILE* outf = fopen(outpath.toLatin1().data(), "wb");
    if (!outf) {
        printf("Unable to open output file for writing.\n");
        fclose(inf);
        return -1;
    }

    qDebug() << "ssfeatures" << inpath << outpath << runparams["nfeatures"].toInt() << runparams["niterations"].toInt(); //okay
    if (!ssfeatures(inf, outf, runparams)) {
        qCritical() << "Problem in ssfeatures.";
    }

    fclose(inf);
    fclose(outf);

    return 0;
}
