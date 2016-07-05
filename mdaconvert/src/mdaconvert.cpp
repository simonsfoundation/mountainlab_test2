/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 7/4/2016
*******************************************************/

#include "mdaconvert.h"
#include "mdaio.h"

#include <QFile>
#include <QFileInfo>
#include <QTime>
#include <stdio.h>

int get_mda_dtype(QString format);

struct working_data {
    ~working_data()
    {
        if (inf)
            fclose(inf);
        if (outf)
            fclose(outf);
        if (buf)
            free(buf);
    }

    MDAIO_HEADER HH_in;
    MDAIO_HEADER HH_out;
    FILE* inf = 0;
    FILE* outf = 0;
    void* buf = 0;
};

bool copy_data(working_data& D, long N);
int get_num_bytes_per_entry(int dtype);

bool mdaconvert(const mdaconvert_opts& opts)
{
    QList<long> opts_dims = opts.dims;

    //default inputs in case input format is mda or not
    if (opts.input_format == "mda") {
        if (!opts.input_dtype.isEmpty()) {
            qWarning() << "input-dtype should not be specified for input type mda";
            return -1;
        }
        if (!opts.dims.isEmpty()) {
            qWarning() << "dims should not be specified for input type mda";
            return -1;
        }
    }
    else {
        if (opts.input_dtype.isEmpty()) {
            qWarning() << "Input datatype must be specified";
            return false;
        }
        if (opts.dims.isEmpty()) {
            qWarning() << "Dimensions must be specified";
            return false;
        }
    }

    //check file existence, etc
    if (!QFile::exists(opts.input_path)) {
        qWarning() << "Input file does not exist: " + opts.input_path;
        return false;
    }
    if (QFile::exists(opts.output_path)) {
        if (!QFile::remove(opts.output_path)) {
            qWarning() << "Unable to remove output file: " + opts.output_path;
            return false;
        }
    }

    // initialize working data
    working_data D;
    for (int i = 0; i < MDAIO_MAX_DIMS; i++) {
        D.HH_in.dims[i] = 1;
        D.HH_out.dims[i] = 1;
    }

    // open input file, read header if mda
    D.inf = fopen(opts.input_path.toLatin1().data(), "rb");
    if (!D.inf) {
        qWarning() << "Unable to open input file for reading: " + opts.input_path;
        return false;
    }
    // read header for format = mda
    if (opts.input_format == "mda") {
        if (!mda_read_header(&D.HH_in, D.inf)) {
            qWarning() << "Error reading input header";
            return false;
        }
    }
    else {
        D.HH_in.data_type = get_mda_dtype(opts.input_dtype);
        D.HH_in.header_size = 0;
        D.HH_in.num_bytes_per_entry = get_num_bytes_per_entry(D.HH_in.data_type);
        if (!D.HH_in.num_bytes_per_entry) {
            qWarning() << "Unable to determine input number of bytes per entry.";
            return false;
        }
        if (opts_dims.value(opts_dims.count() - 1) == 0) {
            //auto-determine final dimension
            long size0 = QFileInfo(opts.input_path).size();
            long tmp = size0 / D.HH_in.num_bytes_per_entry;
            for (int j = 0; j < opts_dims.count() - 1; j++) {
                tmp /= opts_dims[j];
            }
            printf("Auto-setting final dimension to %ld\n", tmp);
            opts_dims[opts_dims.count() - 1] = tmp;
        }
        D.HH_in.num_dims = opts_dims.count();
        for (int i = 0; i < D.HH_in.num_dims; i++) {
            D.HH_in.dims[i] = opts_dims[i];
        }
    }

    if (D.HH_in.num_dims < 2) {
        qWarning() << "Number of dimensions must be at least 2.";
        return false;
    }
    if (D.HH_in.num_dims > 6) {
        qWarning() << "Number of dimensions can be at most 6.";
        return false;
    }

    // output header
    D.HH_out.data_type = D.HH_in.data_type;
    if (!opts.output_dtype.isEmpty()) {
        D.HH_out.data_type = get_mda_dtype(opts.output_dtype);
        if (!D.HH_out.data_type) {
            qWarning() << "Invalid output datatype:" << opts.output_dtype;
            return false;
        }
    }
    D.HH_out.num_bytes_per_entry = get_num_bytes_per_entry(D.HH_out.data_type);
    D.HH_out.num_dims = D.HH_in.num_dims;
    long dim_prod = 1;
    for (int i = 0; i < D.HH_out.num_dims; i++) {
        D.HH_out.dims[i] = D.HH_in.dims[i];
        dim_prod *= D.HH_in.dims[i];
    }

    //open output file, write header if mda
    D.outf = fopen(opts.output_path.toLatin1().data(), "wb");
    if (!D.outf) {
        qWarning() << "Unable to open output file for writing: " + opts.output_path;
        return false;
    }
    if (opts.output_format == "mda") {
        if (!mda_write_header(&D.HH_out, D.outf)) {
            qWarning() << "Error writing output header";
            return false;
        }
    }

    //check input file size
    if (opts.check_input_file_size) {
        long expected_input_file_size = D.HH_in.num_bytes_per_entry * dim_prod + D.HH_in.header_size;
        long actual_input_file_size = QFileInfo(opts.input_path).size();
        if (actual_input_file_size != expected_input_file_size) {
            qWarning() << QString("Unexpected input file size: Expected/Actual=%1/%2 bytes").arg(expected_input_file_size).arg(actual_input_file_size);
            return false;
        }
    }

    bool ret = true;
    long chunk_size = 1e7;
    QTime timer;
    timer.start();
    for (long ii = 0; ii < dim_prod; ii += chunk_size) {
        if (timer.elapsed() > 1000) {
            int pct = (int)((ii * 1.0 / dim_prod) * 100);
            printf("%d%%\n", pct);
            timer.restart();
        }
        long NN = qMin(chunk_size, dim_prod - ii);
        if (!copy_data(D, NN)) {
            ret = false;
            break;
        }
    }

    if (!ret) {
        fclose(D.outf);
        D.outf = 0;
        QFile::remove(opts.output_path);
    }
    return false;
}

bool copy_data(working_data& D, long N)
{
    if (!N)
        return true;
    if (D.buf) {
        free(D.buf);
        D.buf = 0;
    }
    long buf_size = N * D.HH_out.num_bytes_per_entry;
    D.buf = malloc(buf_size);
    if (!D.buf) {
        qWarning() << QString("Error in malloc of size %1 ").arg(buf_size);
        return false;
    }

    void* buf = D.buf;
    long num_read = 0;
    if (D.HH_out.data_type == MDAIO_TYPE_BYTE) {
        num_read = mda_read_byte((unsigned char*)buf, &D.HH_in, N, D.inf);
    }
    else if (D.HH_out.data_type == MDAIO_TYPE_UINT16) {
        num_read = mda_read_uint16((quint16*)buf, &D.HH_in, N, D.inf);
    }
    else if (D.HH_out.data_type == MDAIO_TYPE_INT16) {
        num_read = mda_read_int16((qint16*)buf, &D.HH_in, N, D.inf);
    }
    else if (D.HH_out.data_type == MDAIO_TYPE_INT32) {
        num_read = mda_read_int32((qint32*)buf, &D.HH_in, N, D.inf);
    }
    else if (D.HH_out.data_type == MDAIO_TYPE_FLOAT32) {
        num_read = mda_read_float32((float*)buf, &D.HH_in, N, D.inf);
    }
    else if (D.HH_out.data_type == MDAIO_TYPE_FLOAT64) {
        num_read = mda_read_float64((double*)buf, &D.HH_in, N, D.inf);
    }
    else {
        qWarning() << "Unsupported format for reading";
        return false;
    }

    if (num_read != N) {
        qWarning() << "Error reading data" << num_read << N;
        return false;
    }

    long num_written = 0;
    if (D.HH_out.data_type == MDAIO_TYPE_BYTE) {
        num_written = mda_write_byte((unsigned char*)buf, &D.HH_out, N, D.outf);
    }
    else if (D.HH_out.data_type == MDAIO_TYPE_UINT16) {
        num_written = mda_write_uint16((quint16*)buf, &D.HH_out, N, D.outf);
    }
    else if (D.HH_out.data_type == MDAIO_TYPE_INT16) {
        num_written = mda_write_int16((qint16*)buf, &D.HH_out, N, D.outf);
    }
    else if (D.HH_out.data_type == MDAIO_TYPE_INT32) {
        num_written = mda_write_int32((qint32*)buf, &D.HH_out, N, D.outf);
    }
    else if (D.HH_out.data_type == MDAIO_TYPE_FLOAT32) {
        num_written = mda_write_float32((float*)buf, &D.HH_out, N, D.outf);
    }
    else if (D.HH_out.data_type == MDAIO_TYPE_FLOAT64) {
        num_written = mda_write_float64((double*)buf, &D.HH_out, N, D.outf);
    }
    else {
        qWarning() << "Unsupported format for writing";
        return false;
    }

    if (num_written != N) {
        qWarning() << "Error writing data" << num_written << N;
        return false;
    }

    return true;
}

int get_mda_dtype(QString format)
{
    if (format == "byte")
        return MDAIO_TYPE_BYTE;
    if (format == "int8")
        return MDAIO_TYPE_BYTE;
    if (format == "int16")
        return MDAIO_TYPE_INT16;
    if (format == "int32")
        return MDAIO_TYPE_INT32;
    if (format == "uint16")
        return MDAIO_TYPE_UINT16;
    if (format == "float32")
        return MDAIO_TYPE_FLOAT32;
    if (format == "float64")
        return MDAIO_TYPE_FLOAT64;
    return 0;
}

int get_num_bytes_per_entry(int dtype)
{
    switch (dtype) {
    case MDAIO_TYPE_BYTE:
        return 1;
        break;
    case MDAIO_TYPE_UINT16:
        return 2;
        break;
    case MDAIO_TYPE_INT16:
        return 2;
        break;
    case MDAIO_TYPE_INT32:
        return 4;
        break;
    case MDAIO_TYPE_FLOAT32:
        return 4;
        break;
    case MDAIO_TYPE_FLOAT64:
        return 8;
        break;
    }
    return 0;
}
