/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 8/4/2016
*******************************************************/

#include "diskreadmda_common_impl.h"

#include <QCryptographicHash>
#include "cachemanager.h"
#include <QFile>
#include "taskprogress.h"

#define DEFAULT_CHUNK_SIZE 1e6

void DiskReadMda_Common_Impl::copy_from(DiskReadMda_Common_Impl* other)
{
    if (other->m_file) {
        fclose(other->m_file);
        this->m_file = 0;
    }
    this->construct_and_clear();
    this->m_current_internal_chunk_index = -1;
    this->m_file_open_failed = other->m_file_open_failed;
    this->m_header = other->m_header;
    this->m_header_read = other->m_header_read;
    this->m_mda_header_total_size = other->m_mda_header_total_size;
    this->m_memory_mda_32 = other->m_memory_mda_32;
    this->m_memory_mda_64 = other->m_memory_mda_64;
    this->m_path = other->m_path;
#ifdef USE_REMOTE_READ_MDA
    this->m_remote_mda = other->m_remote_mda;
#endif
    this->m_reshaped = other->m_reshaped;
    this->m_use_memory_mda_32 = other->m_use_memory_mda_32;
    this->m_use_memory_mda_64 = other->m_use_memory_mda_64;
#ifdef USE_REMOTE_READ_MDA
    this->m_use_remote_mda = other->m_use_remote_mda;
#endif
}

void DiskReadMda_Common_Impl::construct_and_clear()
{
    m_file_open_failed = false;
    m_file = 0;
    m_current_internal_chunk_index = -1;
    m_use_memory_mda_32 = false;
    m_use_memory_mda_64 = false;
    m_header_read = false;
    m_reshaped = false;
#ifdef USE_REMOTE_READ_MDA
    m_use_remote_mda = false;
#endif
    this->m_internal_chunk_32 = Mda32();
    this->m_internal_chunk_64 = Mda();
    this->m_mda_header_total_size = 0;
    this->m_memory_mda_32 = Mda32();
    this->m_memory_mda_64 = Mda();
    this->m_path = "";
#ifdef USE_REMOTE_READ_MDA
    this->m_remote_mda = RemoteReadMda();
#endif
}

bool DiskReadMda_Common_Impl::read_header_if_needed()
{
    if (m_header_read)
        return true;
#ifdef USE_REMOTE_READ_MDA
    if (m_use_remote_mda) {
        m_header_read = true;
        return true;
    }
#endif
    if ((m_use_memory_mda_32) || (m_use_memory_mda_64)) {
        m_header_read = true;
        return true;
    }
    bool file_was_open = (m_file != 0); //so we can restore to previous state (we don't want too many files open unnecessarily)
    if (!open_file_if_needed()) //if successful, it will read the header
        return false;
    if (!m_file)
        return false; //should never happen
    if (!file_was_open) {
        fclose(m_file);
        m_file = 0;
    }
    return true;
}

bool DiskReadMda_Common_Impl::open_file_if_needed()
{
#ifdef USE_REMOTE_READ_MDA
    if (m_use_remote_mda)
        return true;
#endif
    if ((m_use_memory_mda_32) || (m_use_memory_mda_64))
        return true;
    if (m_file)
        return true;
    if (m_file_open_failed)
        return false;
    if (m_path.isEmpty())
        return false;
    m_file = fopen(m_path.toLatin1().data(), "rb");
    if (m_file) {
        if (!m_header_read) {
            //important not to read it again in case we have reshaped the array
            mda_read_header(&m_header, m_file);
            m_mda_header_total_size = 1;
            for (int i = 0; i < MDAIO_MAX_DIMS; i++)
                m_mda_header_total_size *= m_header.dims[i];
            m_header_read = true;
        }
    }
    else {
        printf("Failed to open diskreadmda file: %s\n", m_path.toLatin1().data());
        m_file_open_failed = true; //we don't want to try this more than once
        return false;
    }
    return true;
}

long DiskReadMda_Common_Impl::total_size()
{
    if (m_use_memory_mda_32)
        return m_memory_mda_32.totalSize();
    if (m_use_memory_mda_64)
        return m_memory_mda_64.totalSize();
#ifdef USE_REMOTE_READ_MDA
    if (m_use_remote_mda)
        return m_remote_mda.N1() * m_remote_mda.N2() * m_remote_mda.N3();
#endif
    if (!read_header_if_needed())
        return 0;
    return m_mda_header_total_size;
}

void DiskReadMda_Common_Impl::set_remote_data_type(QString dtype)
{
#ifdef USE_REMOTE_READ_MDA
    m_remote_mda.setRemoteDataType(dtype);
#else
    Q_UNUSED(dtype)
#endif
}

void DiskReadMda_Common_Impl::set_download_chunk_size(long size)
{
#ifdef USE_REMOTE_READ_MDA
    m_remote_mda.setDownloadChunkSize(size);
#else
    Q_UNUSED(size)
#endif
}

long DiskReadMda_Common_Impl::download_chunk_size()
{
#ifdef USE_REMOTE_READ_MDA
    return m_remote_mda.downloadChunkSize();
#else
    return 0;
#endif
}

QString compute_memory_checksum(long nbytes, void* ptr)
{
    QByteArray X((char*)ptr, nbytes);
    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(X);
    return QString(hash.result().toHex());
}

QString compute_mda_checksum(Mda32& X)
{
    QString ret = compute_memory_checksum(X.totalSize() * sizeof(float), X.floatPtr());
    ret += "-";
    for (int i = 0; i < X.ndims(); i++) {
        if (i > 0)
            ret += "x";
        ret += QString("%1").arg(X.size(i));
    }
    return ret;
}

QString compute_mda_checksum(Mda& X)
{
    QString ret;
    if (X.isFloat())
        ret=compute_memory_checksum(X.totalSize() * sizeof(float), X.floatPtr());
    else if (X.isDouble())
        ret=compute_memory_checksum(X.totalSize() * sizeof(double), X.doublePtr());
    ret += "-";
    for (int i = 0; i < X.ndims(); i++) {
        if (i > 0)
            ret += "x";
        ret += QString("%1").arg(X.size(i));
    }
    return ret;
}

QString DiskReadMda_Common_Impl::make_path()
{
    if ((m_use_memory_mda_32) || (m_use_memory_mda_64)) {
        QString checksum;
        if (m_use_memory_mda_32)
            checksum = compute_mda_checksum(m_memory_mda_32);
        else
            checksum = compute_mda_checksum(m_memory_mda_64);
        QString fname = CacheManager::globalInstance()->makeLocalFile(checksum + ".makePath.mda", CacheManager::ShortTerm);
        if (QFile::exists(fname))
            return fname;
        if (m_use_memory_mda_32) {
            if (m_memory_mda_32.write32(fname + ".tmp")) {
                if (QFile::rename(fname + ".tmp", fname)) {
                    return fname;
                }
            }
        }
        else {
            if (m_memory_mda_64.write64(fname + ".tmp")) {
                if (QFile::rename(fname + ".tmp", fname)) {
                    return fname;
                }
            }
        }
        QFile::remove(fname);
        QFile::remove(fname + ".tmp");
    }
#ifdef USE_REMOTE_READ_MDA
    if (m_use_remote_mda) {
        return m_remote_mda.makePath();
    }
#endif
    return m_path;
}

long DiskReadMda_Common_Impl::dim(int num)
{
    if (m_use_memory_mda_32) {
        return m_memory_mda_32.dim(num);
    }
    if (m_use_memory_mda_64) {
        return m_memory_mda_64.dim(num);
    }
#ifdef USE_REMOTE_READ_MDA
    if (m_use_remote_mda) {
        return m_remote_mda.dim(num);
    }
#endif
    if (!read_header_if_needed()) {
        return 0;
    }
    return m_header.dims[num - 1];
}

bool DiskReadMda_Common_Impl::reshape(int N1b, int N2b, int N3b, int N4b, int N5b, int N6b)
{
    long size_b = N1b * N2b * N3b * N4b * N5b * N6b;
    if (size_b != total_size()) {
        qWarning() << "Cannot reshape because sizes do not match" << total_size() << N1b << N2b << N3b << N4b << N5b << N6b;
        return false;
    }
    if (m_use_memory_mda_32) {
        if (m_memory_mda_32.reshape(N1b, N2b, N3b, N4b, N5b, N6b)) {
            m_reshaped = true;
            return true;
        }
        else
            return false;
    }
    if (m_use_memory_mda_64) {
        if (m_memory_mda_64.reshape(N1b, N2b, N3b, N4b, N5b, N6b)) {
            m_reshaped = true;
            return true;
        }
        else
            return false;
    }
#ifdef USE_REMOTE_READ_MDA
    if (m_use_remote_mda) {
        if ((N4b != 1) || (N5b != 1) || (N6b != 1)) {
            qWarning() << "Cannot reshape... remote mda can only have 3 dimensions, at this time.";
            return false;
        }
        if (m_remote_mda.reshape(N1b, N2b, N3b)) {
            m_reshaped = true;
            return true;
        }
        else
            return false;
    }
#endif
    if (!read_header_if_needed()) {
        return false;
    }
    m_header.dims[0] = N1b;
    m_header.dims[1] = N2b;
    m_header.dims[2] = N3b;
    m_header.dims[3] = N4b;
    m_header.dims[4] = N5b;
    m_header.dims[5] = N6b;
    m_reshaped = true;
    return true;
}

double DiskReadMda_Common_Impl::value64(long i)
{
    if (m_use_memory_mda_64)
        return m_memory_mda_64.value(i);
    if ((i < 0) || (i >= total_size()))
        return 0;
    long chunk_index = i / DEFAULT_CHUNK_SIZE;
    long offset = i - DEFAULT_CHUNK_SIZE * chunk_index;
    if (m_current_internal_chunk_index != chunk_index) {
        long size_to_read = DEFAULT_CHUNK_SIZE;
        if (chunk_index * DEFAULT_CHUNK_SIZE + size_to_read > total_size())
            size_to_read = total_size() - chunk_index * DEFAULT_CHUNK_SIZE;
        if (size_to_read) {
            this->read_chunk_64(m_internal_chunk_64, chunk_index * DEFAULT_CHUNK_SIZE, size_to_read);
        }
        m_current_internal_chunk_index = chunk_index;
    }
    return m_internal_chunk_64.value(offset);
}

bool DiskReadMda_Common_Impl::read_chunk_64(Mda& X, long i, long size)
{
    if (m_use_memory_mda_64) {
        m_memory_mda_64.getChunk(X, i, size);
        return true;
    }
#ifdef USE_REMOTE_READ_MDA
    if (m_use_remote_mda) {
        return m_remote_mda.readChunk(X, i, size);
    }
#endif
    if (!open_file_if_needed())
        return false;
    X.allocate64(size, 1);
    long jA = qMax(i, 0L);
    long jB = qMin(i + size - 1, total_size() - 1);
    long size_to_read = jB - jA + 1;
    if (size_to_read > 0) {
        fseek(m_file, m_header.header_size + m_header.num_bytes_per_entry * (jA), SEEK_SET);
        long bytes_read = mda_read_float64(&X.doublePtr()[jA - i], &m_header, size_to_read, m_file);
        TaskManager::TaskProgressMonitor::globalInstance()->incrementQuantity("bytes_read", bytes_read);
        if (bytes_read != size_to_read) {
            printf("Warning problem reading chunk in diskreadmda: %ld<>%ld\n", bytes_read, size_to_read);
            return false;
        }
    }
    return true;
}

bool DiskReadMda_Common_Impl::read_chunk_64(Mda& X, long i1, long i2, long size1, long size2)
{
    if (size2 == 0) { //what's this all about ????
        return read_chunk_64(X, i1, size1);
    }
    if (m_use_memory_mda_64) {
        m_memory_mda_64.getChunk(X, i1, i2, size1, size2);
        return true;
    }
#ifdef USE_REMOTE_READ_MDA
    if (m_use_remote_mda) {
        if ((size1 != dim(1)) || (i1 != 0)) {
            qWarning() << "Cannot handle this case yet.";
            return false;
        }
        Mda tmp;
        if (!m_remote_mda.readChunk(tmp, i2 * size1, size1 * size2))
            return false;
        X.allocate64(size1, size2);
        double* Xptr = X.doublePtr();
        double* tmp_ptr = tmp.doublePtr();
        for (long i = 0; i < size1 * size2; i++) {
            Xptr[i] = tmp_ptr[i];
        }
        return true;
    }
#endif
    if (!open_file_if_needed())
        return false;
    if ((size1 == dim(1)) && (i1 == 0)) {
        //easy case
        X.allocate64(size1, size2);
        long jA = qMax(i2, 0L);
        long jB = qMin(i2 + size2 - 1, dim(2) - 1);
        long size2_to_read = jB - jA + 1;
        if (size2_to_read > 0) {
            fseek(m_file, m_header.header_size + m_header.num_bytes_per_entry * (i1 + dim(1) * jA), SEEK_SET);
            long bytes_read = mda_read_float64(&X.doublePtr()[(jA - i2) * size1], &m_header, size1 * size2_to_read, m_file);
            TaskManager::TaskProgressMonitor::globalInstance()->incrementQuantity("bytes_read", bytes_read);
            if (bytes_read != size1 * size2_to_read) {
                printf("Warning problem reading 2d chunk in diskreadmda: %ld<>%ld\n", bytes_read, size1 * size2);
                return false;
            }
        }
        return true;
    }
    else {
        printf("Warning: This case not yet supported (diskreadmda::readchunk 2d).\n");
        return false;
    }
}

bool DiskReadMda_Common_Impl::read_chunk_64(Mda& X, long i1, long i2, long i3, long size1, long size2, long size3)
{
    if (size3 == 0) { //what's this all about?
        if (size2 == 0) {
            return read_chunk_64(X, i1, size1);
        }
        else {
            return read_chunk_64(X, i1, i2, size1, size2);
        }
    }
    if (m_use_memory_mda_64) {
        m_memory_mda_64.getChunk(X, i1, i2, i3, size1, size2, size3);
        return true;
    }
#ifdef USE_REMOTE_READ_MDA
    if (m_use_remote_mda) {
        if ((size1 != dim(1)) || (i1 != 0) || (size2 != dim(2)) || (i2 != 0)) {
            qWarning() << "Cannot handle this case yet **." << size1 << dim(1) << i1 << size2 << dim(2) << i2 << this->m_path;
            return false;
        }

        Mda tmp;
        if (!m_remote_mda.readChunk(tmp, i3 * size1 * size2, size1 * size2 * size3))
            return false;
        X.allocate64(size1, size2, size3);
        double* Xptr = X.doublePtr();
        double* tmp_ptr = tmp.doublePtr();
        for (long i = 0; i < size1 * size2 * size3; i++) {
            Xptr[i] = tmp_ptr[i];
        }
        return true;
    }
#endif
    if (!open_file_if_needed())
        return false;
    if ((size1 == dim(1)) && (size2 == dim(2))) {
        //easy case
        X.allocate64(size1, size2, size3);
        long jA = qMax(i3, 0L);
        long jB = qMin(i3 + size3 - 1, dim(3) - 1);
        long size3_to_read = jB - jA + 1;
        if (size3_to_read > 0) {
            fseek(m_file, m_header.header_size + m_header.num_bytes_per_entry * (i1 + dim(1) * i2 + dim(1) * dim(2) * jA), SEEK_SET);
            long bytes_read = mda_read_float64(&X.doublePtr()[(jA - i3) * size1 * size2], &m_header, size1 * size2 * size3_to_read, m_file);
            TaskManager::TaskProgressMonitor::globalInstance()->incrementQuantity("bytes_read", bytes_read);
            if (bytes_read != size1 * size2 * size3_to_read) {
                printf("Warning problem reading 3d chunk in diskreadmda: %ld<>%ld\n", bytes_read, size1 * size2 * size3_to_read);
                return false;
            }
        }
        return true;
    }
    else {
        printf("Warning: This case not yet supported (diskreadmda::readchunk 3d).\n");
        return false;
    }
}
