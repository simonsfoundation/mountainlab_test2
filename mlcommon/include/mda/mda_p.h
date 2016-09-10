#ifndef MDA_P_H
#define MDA_P_H

#include <QSharedData>
#include "taskprogress.h"
#include <cstring>
#include "mlcommon.h"

#define MDA_MAX_DIMS 6

template<typename T>
class MdaData : public QSharedData {
public:
    typedef T value_type;
    typedef T* pointer;
    typedef T& reference;

    MdaData() : QSharedData(), m_data(0), m_dims(1,1), total_size(0) {}
    MdaData(const MdaData &other)
        : QSharedData(other), m_data(0), m_dims(other.m_dims), total_size(other.total_size) {
        allocate(total_size);
        std::copy(other.m_data, other.m_data+other.totalSize(), m_data);
    }
    ~MdaData() {
        deallocate();
    }
    bool allocate(T value, long N1, long N2, long N3 = 1, long N4 = 1, long N5 = 1, long N6 = 1) {
        deallocate();
        setDims(N1, N2, N3, N4, N5, N6);
        if (N1 > 0 && N2 > 0 && N3 > 0 && N4 > 0 && N5 > 0 && N6 > 0)
            setTotalSize(N1 * N2 * N3 * N4 * N5 * N6);
        else setTotalSize(0);

        if (totalSize() > 0) {
            allocate(totalSize());
            if (!constData()) {
                qCritical() << QString("Unable to allocate Mda of size %1x%2x%3x%4x%5x%6 (total=%7)").arg(N1).arg(N2).arg(N3).arg(N4).arg(N5).arg(N6).arg(totalSize());
                exit(-1);
            }
            if (value == 0.0) {
                std::memset(data(), 0, totalSize()*sizeof(value_type));
            } else
                std::fill(data(), data()+totalSize(), value);
        }
        return true;
    }

    inline long dim(size_t idx) const { return m_dims.at(idx); }
    inline long N1() const { return dim(0); }
    inline long N2() const { return dim(1); }

    void allocate(size_t size) {
        m_data = (value_type*)::allocate(size*sizeof(value_type));
        if (!m_data) return;
        TaskManager::TaskProgressMonitor::globalInstance()->incrementQuantity("bytes_allocated", totalSize()*sizeof(value_type));
    }
    void deallocate() {
        if (!m_data) return;
        free(m_data);
        TaskManager::TaskProgressMonitor::globalInstance()->incrementQuantity("bytes_freed", totalSize()*sizeof(value_type));
        m_data = 0;
    }
    inline size_t totalSize() const { return total_size; }
    inline void setTotalSize(size_t ts) { total_size = ts; }
    inline T *data() { return m_data; }
    inline const T* constData() const { return m_data; }
    inline T at(size_t idx) const { return *(constData()+idx); }
    inline T at(size_t i1, size_t i2) const { return at(i1 + dim(0) * i2); }
    inline void set(T val, size_t idx) { m_data[idx] = val; }
    inline void set(T val, size_t i1, size_t i2) { set(val, i1 + dim(0) * i2); }

    inline long dims(size_t idx) const
    {
        if (idx < 0 || idx >= m_dims.size()) return 0;
        return *(m_dims.data()+idx);
    }
    void setDims(long n1, long n2, long n3, long n4, long n5, long n6)
    {
        m_dims.resize(MDA_MAX_DIMS);
        m_dims[0] = n1;
        m_dims[1] = n2;
        m_dims[2] = n3;
        m_dims[3] = n4;
        m_dims[4] = n5;
        m_dims[5] = n6;
    }

    int determine_num_dims(long N1, long N2, long N3, long N4, long N5, long N6) const
    {
        if (!(N6 > 0 && N5 > 0 && N4 > 0 && N3 > 0 && N2 > 0 && N1 > 0)) return 0;
        if (N6 > 1)
            return 6;
        if (N5 > 1)
            return 5;
        if (N4 > 1)
            return 4;
        if (N3 > 1)
            return 3;
        return 2;
    }
    bool safe_index(size_t i) const
    {
        return (i < totalSize());
    }
    bool safe_index(size_t i1, size_t i2) const
    {
        return (((long)i1 < dims(0)) && ((long)i2 < dims(1)));
    }
    bool safe_index(size_t i1, size_t i2, size_t i3) const
    {
        return (((long)i1 < dims(0)) && ((long)i2 < dims(1)) && ((long)i3 < dims(2)));
    }
    bool safe_index(long i1, long i2, long i3, long i4, long i5, long i6) const
    {
        return (
                    (0 <= i1) && (i1 < dims(0))
                    && (0 <= i2) && (i2 < dims(1))
                    && (0 <= i3) && (i3 < dims(2))
                    && (0 <= i4) && (i4 < dims(3))
                    && (0 <= i5) && (i5 < dims(4))
                    && (0 <= i6) && (i6 < dims(5))
                    );
    }

    bool read_from_text_file(const QString& path)
    {
        QString txt = TextFile::read(path);
        if (txt.isEmpty()) {
            return false;
        }
        QStringList lines = txt.split("\n", QString::SkipEmptyParts);
        QStringList lines2;
        for (int i = 0; i < lines.count(); i++) {
            QString line = lines[i].trimmed();
            if (!line.isEmpty()) {
                if (i == 0) {
                    //check whether this is a header line, if so, don't include it
                    line = line.split(",", QString::SkipEmptyParts).join(" ");
                    QList<QString> vals = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
                    bool ok;
                    vals.value(0).toDouble(&ok);
                    if (ok) {
                        lines2 << line;
                    }
                }
                else {
                    lines2 << line;
                }
            }
        }
        for (int i = 0; i < lines2.count(); i++) {
            QString line = lines2[i].trimmed();
            line = line.split(",", QString::SkipEmptyParts).join(" ");
            QList<QString> vals = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
            if (i == 0) {
                allocate(0, vals.count(), lines2.count());
            }
            for (int j = 0; j < vals.count(); j++) {
                set(vals[j].toDouble(), j, i);
            }
        }
        return true;
    }
    bool write_to_text_file(const QString& path) const
    {
        char sep = ' ';
        if (path.endsWith(".csv"))
            sep = ',';
        long max_num_entries = 1e6;
        if (N1() * N2() == max_num_entries) {
            qWarning() << "mda is too large to write text file";
            return false;
        }
        QList<QString> lines;
        for (long i = 0; i < N2(); i++) {
            QStringList vals;
            for (long j = 0; j < N1(); j++) {
                vals << QString("%1").arg(at(j, i));
            }
            QString line = vals.join(sep);
            lines << line;
        }
        return TextFile::write(path, lines.join("\n"));
    }

private:
    pointer m_data;
    std::vector<long> m_dims;
    size_t total_size;
};


#endif // MDA_P_H
