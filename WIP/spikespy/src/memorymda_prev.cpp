#include "memorymda_prev.h"
#include "mdaio.h"
#include <QDebug>
#include "usagetracking.h"

class MemoryMdaPrivate {
public:
    MemoryMda* q;
    int m_size[MDAIO_MAX_DIMS];
    double* m_data;
    int m_total_size;

    int get_index(int i1, int i2, int i3, int i4, int i5, int i6);
};

MemoryMda::MemoryMda(QObject* parent)
    : QObject(parent)
{
    d = new MemoryMdaPrivate;
    d->q = this;
    for (int i = 0; i < MDAIO_MAX_DIMS; i++)
        d->m_size[i] = 1;
    d->m_data = (double*)jmalloc(sizeof(double) * 1);
    d->m_total_size = 1;
}

MemoryMda::MemoryMda(const MemoryMda& other)
    : QObject()
{
    d = new MemoryMdaPrivate;
    d->q = this;

    for (int i = 0; i < MDAIO_MAX_DIMS; i++)
        d->m_size[i] = other.d->m_size[i];
    d->m_data = (double*)jmalloc(sizeof(double) * other.d->m_total_size);
    d->m_total_size = other.d->m_total_size;

    for (int i = 0; i < d->m_total_size; i++) {
        d->m_data[i] = other.d->m_data[i];
    }
}
void MemoryMda::operator=(const MemoryMda& other)
{
    if (d->m_data)
        jfree(d->m_data);

    for (int i = 0; i < MDAIO_MAX_DIMS; i++)
        d->m_size[i] = other.d->m_size[i];
    d->m_data = (double*)jmalloc(sizeof(double) * other.d->m_total_size);
    d->m_total_size = other.d->m_total_size;

    for (int i = 0; i < d->m_total_size; i++) {
        d->m_data[i] = other.d->m_data[i];
    }
}

int MemoryMda::size(int dim) const
{
    return d->m_size[dim];
}

int MemoryMda::N1() const { return d->m_size[0]; }
int MemoryMda::N2() const { return d->m_size[1]; }
int MemoryMda::N3() const { return d->m_size[2]; }
int MemoryMda::N4() const { return d->m_size[3]; }
int MemoryMda::N5() const { return d->m_size[4]; }
int MemoryMda::N6() const { return d->m_size[5]; }

void MemoryMda::write(const QString& path)
{
    //Always write type float32!!!!!!!!!!!!!
    FILE* outf = jfopen(path.toLatin1().data(), "wb");
    if (!outf) {
        qWarning() << "unable to open file for writing..." << path;
        return;
    }
    MDAIO_HEADER HH;
    HH.data_type = MDAIO_TYPE_FLOAT32;
    for (int i = 0; i < MDAIO_MAX_DIMS; i++)
        HH.dims[i] = d->m_size[i];
    HH.num_bytes_per_entry = 4;
    int num_dims = 2;
    for (int i = 3; i < MDAIO_MAX_DIMS; i++) {
        if (d->m_size[i] > 1)
            num_dims = i + 1;
    }
    HH.num_dims = num_dims;
    mda_write_header(&HH, outf);
    mda_write_float64(d->m_data, &HH, d->m_total_size, outf);
    jfclose(outf);
}

double MemoryMda::value(int i1, int i2, int i3, int i4, int i5, int i6)
{
    int ind = d->get_index(i1, i2, i3, i4, i5, i6);
    if ((ind < 0) || (ind > d->m_total_size))
        return 0;
    return d->m_data[ind];
}

void MemoryMda::setValue(double val, int i1, int i2, int i3, int i4, int i5, int i6)
{
    int ind = d->get_index(i1, i2, i3, i4, i5, i6);
    if ((ind < 0) || (ind > d->m_total_size))
        return;
    d->m_data[ind] = val;
}

MemoryMda::~MemoryMda()
{
    if (d->m_data)
        jfree(d->m_data);
    delete d;
}

void MemoryMda::allocate(int N1, int N2, int N3, int N4, int N5, int N6)
{
    if (d->m_data)
        jfree(d->m_data);

    int dims[MDAIO_MAX_DIMS];
    for (int i = 0; i < MDAIO_MAX_DIMS; i++)
        dims[i] = 1;
    dims[0] = N1;
    dims[1] = N2;
    dims[2] = N3;
    dims[3] = N4;
    dims[4] = N5;
    dims[5] = N6;
    int num_dims = 2;
    for (int i = 2; i < MDAIO_MAX_DIMS; i++) {
        if (dims[i] > 1)
            num_dims = i + 1;
    }
    d->m_total_size = 1;
    for (int i = 0; i < num_dims; i++) {
        d->m_total_size *= dims[i];
    }
    for (int i = 0; i < MDAIO_MAX_DIMS; i++) {
        d->m_size[i] = dims[i];
    }

    d->m_data = (double*)jmalloc(sizeof(double) * d->m_total_size);
    for (int i = 0; i < d->m_total_size; i++) {
        d->m_data[i] = 0;
    }
}

int MemoryMdaPrivate::get_index(int i1, int i2, int i3, int i4, int i5, int i6)
{
    int inds[6];
    inds[0] = i1;
    inds[1] = i2;
    inds[2] = i3;
    inds[3] = i4;
    inds[4] = i5;
    inds[5] = i6;
    int factor = 1;
    int ret = 0;
    for (int j = 0; j < 6; j++) {
        ret += factor * inds[j];
        factor *= m_size[j];
    }
    return ret;
}
