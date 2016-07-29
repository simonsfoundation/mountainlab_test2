#include "diskreadmdaold.h"

#include <QFile>
#include <QFileInfo>
#include <QVector>
#include "mdaio.h"
#include "usagetracking.h"
#include <math.h>
#include <QDebug>

#define CHUNKSIZE 10000

struct DataChunk {
    double* data;
};

class DiskReadMdaOldPrivate {
public:
    DiskReadMdaOld* q;
    QVector<DataChunk> m_chunks;
    FILE* m_file;
    long m_header_size;
    long m_num_bytes_per_entry;
    long m_total_size;
    QString m_path;
    long m_data_type;
    long m_size[MDAIO_MAX_DIMS];
    Mda m_memory_mda;
    bool m_using_memory_mda;

    long get_index(long i1, long i2, long i3, long i4 = 0, long i5 = 0, long i6 = 0);
    long get_index(long i1, long i2);
    double* load_chunk(long i);
    void clear_chunks();
    void load_header();
    void initialize_contructor();
};

DiskReadMdaOld::DiskReadMdaOld(const QString& path)
    : QObject()
{
    d = new DiskReadMdaOldPrivate;
    d->q = this;
    d->initialize_contructor();

    if (!path.isEmpty())
        setPath(path);
}

DiskReadMdaOld::DiskReadMdaOld(const DiskReadMdaOld& other)
    : QObject()
{
    d = new DiskReadMdaOldPrivate;
    d->q = this;
    d->initialize_contructor();

    if (other.d->m_using_memory_mda) {
        d->m_using_memory_mda = true;
        d->m_memory_mda = other.d->m_memory_mda;
    }
    else {
        setPath(other.d->m_path);
    }
}

DiskReadMdaOld::DiskReadMdaOld(const Mda& X)
{
    d = new DiskReadMdaOldPrivate;
    d->q = this;
    d->initialize_contructor();

    d->m_memory_mda = X;
    d->m_using_memory_mda = true;
}

void DiskReadMdaOld::operator=(const DiskReadMdaOld& other)
{
    if (other.d->m_using_memory_mda) {
        d->m_using_memory_mda = true;
        d->m_memory_mda = other.d->m_memory_mda;
        return;
    }
    setPath(other.d->m_path);
}

DiskReadMdaOld::~DiskReadMdaOld()
{
    d->clear_chunks();
    if (d->m_file)
        jfclose(d->m_file);
    delete d;
}

void DiskReadMdaOld::setPath(const QString& path)
{
    if (path == d->m_path)
        return; //added 3/3/2016
    d->clear_chunks();
    if (d->m_file)
        jfclose(d->m_file);
    d->m_file = 0;
    d->m_path = path;
    d->load_header();
    d->m_chunks.resize(ceil(d->m_total_size * 1.0 / CHUNKSIZE));
    for (long i = 0; i < d->m_chunks.count(); i++) {
        d->m_chunks[i].data = 0;
    }
}

long DiskReadMdaOld::N1() const { return size(0); }
long DiskReadMdaOld::N2() const { return size(1); }
long DiskReadMdaOld::N3() const { return size(2); }
long DiskReadMdaOld::N4() const { return size(3); }
long DiskReadMdaOld::N5() const { return size(4); }
long DiskReadMdaOld::N6() const { return size(5); }

long DiskReadMdaOld::totalSize() const
{
    if (d->m_using_memory_mda) {
        return d->m_memory_mda.totalSize();
    }
    return d->m_total_size;
}

long DiskReadMdaOld::size(long dim) const
{
    if (d->m_using_memory_mda) {
        return d->m_memory_mda.size(dim);
    }
    if (dim >= MDAIO_MAX_DIMS)
        return 1;

    return d->m_size[dim];
}

double DiskReadMdaOld::value(long i1, long i2)
{
    if (d->m_using_memory_mda) {
        return d->m_memory_mda.value(i1, i2);
    }
    long ind = d->get_index(i1, i2);
    if (ind < 0)
        return 0;
    double* X = d->load_chunk(ind / CHUNKSIZE);
    if (!X) {
        //qWarning() << "chunk not loaded:" << ind << ind/CHUNKSIZE;
        return 0;
    }
    return X[ind % CHUNKSIZE];
}

double DiskReadMdaOld::value(long i1, long i2, long i3, long i4, long i5, long i6)
{
    if (d->m_using_memory_mda) {
        return d->m_memory_mda.value(i1, i2, i3, i4);
    }
    long ind = d->get_index(i1, i2, i3, i4, i5, i6);
    if (ind < 0)
        return 0;
    double* X = d->load_chunk(ind / CHUNKSIZE);
    if (!X) {
        //qWarning() << "chunk not loaded:" << ind << ind/CHUNKSIZE;
        return 0;
    }
    return X[ind % CHUNKSIZE];
}

double DiskReadMdaOld::value1(long ind)
{
    if (d->m_using_memory_mda) {
        return d->m_memory_mda.value(ind);
    }
    double* X = d->load_chunk(ind / CHUNKSIZE);
    if (!X)
        return 0;
    return X[ind % CHUNKSIZE];
}

void DiskReadMdaOld::reshape(long N1, long N2, long N3, long N4, long N5, long N6)
{
    if (d->m_using_memory_mda) {
        printf("Warning: unable to reshape because we are using a memory mda.\n");
    }
    long tot = N1 * N2 * N3 * N4 * N5 * N6;
    if (tot != d->m_total_size) {
        printf("Warning: unable to reshape %ld <> %ld: %s\n", tot, d->m_total_size, d->m_path.toLatin1().data());
        return;
    }
    for (long i = 0; i < MDAIO_MAX_DIMS; i++)
        d->m_size[i] = 1;
    d->m_size[0] = N1;
    d->m_size[1] = N2;
    d->m_size[2] = N3;
    d->m_size[3] = N4;
    d->m_size[4] = N5;
    d->m_size[5] = N6;
}

void DiskReadMdaOld::write(const QString& path)
{
    if (d->m_using_memory_mda) {
        d->m_memory_mda.write32(path);
        return;
    }
    if (path == d->m_path)
        return;
    if (path.isEmpty())
        return;
    if (QFileInfo(path).exists()) {
        QFile::remove(path);
    }
    QFile::copy(d->m_path, path);
}

long DiskReadMdaOldPrivate::get_index(long i1, long i2, long i3, long i4, long i5, long i6)
{
    long inds[6];
    inds[0] = i1;
    inds[1] = i2;
    inds[2] = i3;
    inds[3] = i4;
    inds[4] = i5;
    inds[5] = i6;
    long factor = 1;
    long ret = 0;
    for (long j = 0; j < 6; j++) {
        if (inds[j] >= m_size[j])
            return -1;
        ret += factor * inds[j];
        factor *= m_size[j];
    }
    return ret;
}

long DiskReadMdaOldPrivate::get_index(long i1, long i2)
{
    return i1 + m_size[0] * i2;
}

double* DiskReadMdaOldPrivate::load_chunk(long i)
{
    if (i >= m_chunks.count()) {
        //qWarning() << "i>=m_chunks.count()" << this->m_size[0] << this->m_size[1] << this->m_size[2] << this->m_size[3];
        return 0;
    }
    if (!m_chunks[i].data) {
        m_chunks[i].data = (double*)jmalloc(sizeof(double) * CHUNKSIZE);
        for (long j = 0; j < CHUNKSIZE; j++)
            m_chunks[i].data[j] = 0;
        if (m_file) {
            fseek(m_file, m_header_size + m_num_bytes_per_entry * i * CHUNKSIZE, SEEK_SET);
            size_t num;
            if (i * CHUNKSIZE + CHUNKSIZE <= m_total_size)
                num = CHUNKSIZE;
            else {
                num = m_total_size - i * CHUNKSIZE;
            }
            void* data = jmalloc(m_num_bytes_per_entry * num);
            size_t num_read = jfread(data, m_num_bytes_per_entry, num, m_file);
            if (num_read == num) {
                if (m_data_type == MDAIO_TYPE_BYTE) {
                    unsigned char* tmp = (unsigned char*)data;
                    double* tmp2 = m_chunks[i].data;
                    for (size_t j = 0; j < num; j++)
                        tmp2[j] = (double)tmp[j];
                }
                else if (m_data_type == MDAIO_TYPE_FLOAT32) {
                    float* tmp = (float*)data;
                    double* tmp2 = m_chunks[i].data;
                    for (size_t j = 0; j < num; j++)
                        tmp2[j] = (double)tmp[j];
                }
                else if (m_data_type == MDAIO_TYPE_INT16) {
                    int16_t* tmp = (int16_t*)data;
                    double* tmp2 = m_chunks[i].data;
                    for (size_t j = 0; j < num; j++)
                        tmp2[j] = (double)tmp[j];
                }
                else if (m_data_type == MDAIO_TYPE_INT32) {
                    int32_t* tmp = (int32_t*)data;
                    double* tmp2 = m_chunks[i].data;
                    for (size_t j = 0; j < num; j++)
                        tmp2[j] = (double)tmp[j];
                }
                else if (m_data_type == MDAIO_TYPE_UINT16) {
		    quint32* tmp = (quint32*)data;
                    double* tmp2 = m_chunks[i].data;
                    for (size_t j = 0; j < num; j++)
                        tmp2[j] = (double)tmp[j];
                }
                else if (m_data_type == MDAIO_TYPE_FLOAT64) {
                    double* tmp = (double*)data;
                    double* tmp2 = m_chunks[i].data;
                    for (size_t j = 0; j < num; j++)
                        tmp2[j] = (double)tmp[j];
		}
                else {
                    printf("Warning: unexpected data type: %ld\n", m_data_type);
                }
                jfree(data);
            }
            else {
                qWarning() << "Problem reading from mda 212" << num_read << num << m_path << i << m_num_bytes_per_entry << m_size[0] << m_size[1] << m_size[2];
            }
        }
        else
            qWarning() << "File is not open!";
    }
    return m_chunks[i].data;
}

void DiskReadMdaOldPrivate::clear_chunks()
{
    for (long i = 0; i < m_chunks.count(); i++) {
        if (m_chunks[i].data)
            jfree(m_chunks[i].data);
    }
    m_chunks.clear();
}

void DiskReadMdaOldPrivate::load_header()
{
    if (m_file) {
        printf("Unexpected problem in load_header\n");
        return;
    }
    if (m_path.isEmpty()) {
        return;
    }

    if (m_path.contains("/spikespy-server?")) {
    }

    m_file = jfopen(m_path.toLatin1().data(), "rb");
    if (!m_file) {
        printf("Unable to open file %s\n", m_path.toLatin1().data());
        return;
    }
    MDAIO_HEADER HH;
    if (!mda_read_header(&HH, m_file)) {
        qWarning() << "Problem in mda_read_header" << m_path;
    }
    m_data_type = HH.data_type;
    m_num_bytes_per_entry = HH.num_bytes_per_entry;
    m_total_size = 1;
    for (long i = 0; i < MDAIO_MAX_DIMS; i++) {
        m_size[i] = HH.dims[i];
        m_total_size *= m_size[i];
    }
    m_header_size = HH.header_size;
}

void DiskReadMdaOldPrivate::initialize_contructor()
{
    m_file = 0;
    m_header_size = 0;
    m_num_bytes_per_entry = 0;
    m_total_size = 0;
    m_data_type = MDAIO_TYPE_FLOAT32;
    m_using_memory_mda = false;
    for (long i = 0; i < MDAIO_MAX_DIMS; i++)
        m_size[i] = 1;
}
