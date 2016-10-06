/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 7/4/2016
*******************************************************/

#include "mdaconvert.h"
#include "mlcommon.h"

#include <QFile>
#include <diskreadmda.h>

void print_usage();

QString get_default_format(QString path);
QString get_info_string(const DiskReadMda& X);

/// TODO, auto-calculate the last dimension

int main(int argc, char* argv[])
{
    CLParams params(argc, argv);

    if (params.unnamed_parameters.count() < 1) {
        print_usage();
        return 0;
    }

    mdaconvert_opts opts;

    opts.input_path = params.unnamed_parameters.value(0);
    opts.input_dtype = params.named_parameters.value("input-dtype").toString();
    opts.input_format = params.named_parameters.value("input-format").toString();

    opts.output_path = params.unnamed_parameters.value(1);
    opts.output_dtype = params.named_parameters.value("output-dtype").toString();
    opts.output_format = params.named_parameters.value("output-format").toString();

    if (opts.output_path.isEmpty()) {
        if (opts.input_path.endsWith(".mda")) {
            DiskReadMda X(opts.input_path);
            QString str = get_info_string(X);
            printf("%s\n", str.toUtf8().data());
            return 0;
        }
        else {
            print_usage();
            return -1;
        }
    }

    if (params.named_parameters.contains("dtype")) {
        opts.input_dtype = params.named_parameters.value("dtype").toString();
        opts.output_dtype = params.named_parameters.value("dtype").toString();
    }

    if (opts.input_format.isEmpty()) {
        opts.input_format = get_default_format(opts.input_path);
    }
    if (opts.output_format.isEmpty()) {
        opts.output_format = get_default_format(opts.output_path);
    }

    QStringList dims_strlist = params.named_parameters.value("dims", "").toString().split("x", QString::SkipEmptyParts);
    foreach (QString str, dims_strlist) {
        opts.dims << str.toLong();
    }

    printf("Converting %s --> %s\n", opts.input_path.toLatin1().data(), opts.output_path.toLatin1().data());
    if (!mdaconvert(opts)) {
        return -1;
    }

    return 0;
}

QString get_suffix(QString path)
{
    int index1 = path.lastIndexOf(".");
    if (index1 < 0)
        return "";
    int index2 = path.lastIndexOf("/");
    if ((index2 >= 0) && (index2 > index1))
        return "";
    return path.mid(index1 + 1);
}

QString get_default_format(QString path)
{
    QString suf = get_suffix(path);
    if (suf == "mda")
        return "mda";
    else
        return "raw";
}

void print_usage()
{
    printf("Example usages for converting between raw and mda formats:\n");
    printf("mdaconvert input.mda\n");
    printf("mdaconvert input.dat output.mda --dtype=uint16 --dims=32x100x44\n");
    printf("mdaconvert input.mda output.dat\n");
    printf("mdaconvert input.file output.file --input-format=dat --input-dtype=float64 --output-format=mda --output-dtype=float32\n");
}

#define MDAIO_MAX_DIMS 50
#define MDAIO_MAX_SIZE 1e15
#define MDAIO_TYPE_COMPLEX -1
#define MDAIO_TYPE_BYTE -2
#define MDAIO_TYPE_FLOAT32 -3
#define MDAIO_TYPE_INT16 -4
#define MDAIO_TYPE_INT32 -5
#define MDAIO_TYPE_UINT16 -6
#define MDAIO_TYPE_FLOAT64 -7
#define MDAIO_TYPE_UINT32 -8

QString mdaio_type_string(int dtype)
{
    if (dtype == MDAIO_TYPE_COMPLEX)
        return "complex";
    else if (dtype == MDAIO_TYPE_FLOAT32)
        return "float32";
    else if (dtype == MDAIO_TYPE_FLOAT64)
        return "float64";
    else if (dtype == MDAIO_TYPE_BYTE)
        return "byte";
    else if (dtype == MDAIO_TYPE_UINT16)
        return "uint16";
    else if (dtype == MDAIO_TYPE_UINT32)
        return "uint32";
    else if (dtype == MDAIO_TYPE_INT16)
        return "int16";
    else if (dtype == MDAIO_TYPE_INT32)
        return "int32";
    else
        return QString("unknown type %1").arg(dtype);
}

QString get_info_string(const DiskReadMda& X)
{
    QString str = QString("%1x%2").arg(X.N1()).arg(X.N2());
    if (X.N6() > 1) {
        str += QString("x%1x%2x%3x%4").arg(X.N3()).arg(X.N4()).arg(X.N5()).arg(X.N6());
    }
    else if (X.N5() > 1) {
        str += QString("x%1x%2x%3").arg(X.N3()).arg(X.N4()).arg(X.N5());
    }
    else if (X.N4() > 1) {
        str += QString("x%1x%2").arg(X.N3()).arg(X.N4());
    }
    else if (X.N3() > 1) {
        str += QString("x%1").arg(X.N3());
    }
    MDAIO_HEADER header = X.mdaioHeader();
    str += QString(" (%1)").arg(mdaio_type_string(header.data_type));
    return str;
}
