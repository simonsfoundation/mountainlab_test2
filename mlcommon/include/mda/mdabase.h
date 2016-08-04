#ifndef MDABASE_H
#define MDABASE_H

#include <QString>
#include <QDebug>

extern void* allocate(const size_t nbytes);

class MdaBasePrivate;
class MdaBase {
public:

    enum MdaBaseDataType {
        Float,Double
    };

    friend class MdaBasePrivate;
    MdaBase(MdaBaseDataType dtype, long N1 = 1, long N2 = 1, long N3 = 1, long N4 = 1, long N5 = 1, long N6 = 1);
    MdaBase(MdaBaseDataType dtype, const QString mda_filename);
    //MdaBase(const MdaBase& other);
    //void operator=(const MdaBase& other);
    virtual ~MdaBase();
    bool allocate(MdaBaseDataType dtype, long N1, long N2, long N3 = 1, long N4 = 1, long N5 = 1, long N6 = 1);
    bool allocate(long N1, long N2, long N3 = 1, long N4 = 1, long N5 = 1, long N6 = 1);
    bool read(const QString& path);
    bool write8(const QString& path) const;
    bool write32(const QString& path) const;
    bool write64(const QString& path) const;

    QByteArray toByteArray8() const;
    QByteArray toByteArray32() const;
    QByteArray toByteArray64() const;
    bool fromByteArray(const QByteArray& X);

    MdaBaseDataType dataType() const;
    int ndims() const;
    long N1() const;
    long N2() const;
    long N3() const;
    long N4() const;
    long N5() const;
    long N6() const;
    long totalSize() const;
    long size(int dimension_index) const; //zero-based

    float get32(long i) const;
    float get32(long i1, long i2) const;
    float get32(long i1, long i2, long i3) const;
    float get32(long i1, long i2, long i3, long i4, long i5 = 0, long i6 = 0) const;

    double get64(long i) const;
    double get64(long i1, long i2) const;
    double get64(long i1, long i2, long i3) const;
    double get64(long i1, long i2, long i3, long i4, long i5 = 0, long i6 = 0) const;

    void set32(float val, long i);
    void set32(float val, long i1, long i2);
    void set32(float val, long i1, long i2, long i3);
    void set32(float val, long i1, long i2, long i3, long i4, long i5 = 0, long i6 = 0);

    void set64(double val, long i);
    void set64(double val, long i1, long i2);
    void set64(double val, long i1, long i2, long i3);
    void set64(double val, long i1, long i2, long i3, long i4, long i5 = 0, long i6 = 0);

    double value(long i) const;
    double value(long i1, long i2) const;
    double value(long i1, long i2, long i3) const;
    double value(long i1, long i2, long i3, long i4, long i5 = 0, long i6 = 0) const;

    void setValue(double val, long i);
    void setValue(double val, long i1, long i2);
    void setValue(double val, long i1, long i2, long i3);
    void setValue(double val, long i1, long i2, long i3, long i4, long i5 = 0, long i6 = 0);

    float* dataPtr32();
    float* dataPtr32(long i);
    float* dataPtr32(long i1, long i2);
    float* dataPtr32(long i1, long i2, long i3);
    float* dataPtr32(long i1, long i2, long i3, long i4, long i5 = 0, long i6 = 0);
    const float* constDataPtr32() const;

    double* dataPtr64();
    double* dataPtr64(long i);
    double* dataPtr64(long i1, long i2);
    double* dataPtr64(long i1, long i2, long i3);
    double* dataPtr64(long i1, long i2, long i3, long i4, long i5 = 0, long i6 = 0);
    const double* constDataPtr64() const;

    void getChunk(MdaBase& ret, long i, long N);
    void getChunk(MdaBase& ret, long i1, long i2, long N1, long N2);
    void getChunk(MdaBase& ret, long i1, long i2, long i3, long size1, long size2, long size3);

    void setChunk(MdaBase& X, long i);
    void setChunk(MdaBase& X, long i1, long i2);
    void setChunk(MdaBase& X, long i1, long i2, long i3);

    double minimum() const;
    double maximum() const;

    bool reshape(int N1b, int N2b, int N3b = 1, int N4b = 1, int N5b = 1, int N6b = 1);

private:
    MdaBasePrivate* d;
};


#endif // MDABASE_H

