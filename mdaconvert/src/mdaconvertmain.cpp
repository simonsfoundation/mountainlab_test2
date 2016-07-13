/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 7/4/2016
*******************************************************/

#include "mdaconvert.h"
#include "mlcommon.h"

#include <QFile>

void print_usage();

QString get_default_format(QString path);

/// TODO, auto-calculate the last dimension

int main(int argc, char* argv[])
{
    CLParams params(argc, argv);

    if (params.unnamed_parameters.count() != 2) {
        print_usage();
        return 0;
    }

    mdaconvert_opts opts;

    opts.input_path = params.unnamed_parameters[0];
    opts.input_dtype = params.named_parameters.value("input-dtype").toString();
    opts.input_format = params.named_parameters.value("input-format").toString();

    opts.output_path = params.unnamed_parameters[1];
    opts.output_dtype = params.named_parameters.value("output-dtype").toString();
    opts.output_format = params.named_parameters.value("output-format").toString();

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
    printf("mdaconvert input.dat output.mda --dtype=uint16 --dims=32x100x44\n");
    printf("mdaconvert input.mda output.dat\n");
    printf("mdaconvert input.file output.file --input-format=dat --input-dtype=float64 --output-format=mda --output-dtype=float32\n");
}
