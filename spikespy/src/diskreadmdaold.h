#ifndef DISKREADMDAOLD_H
#define DISKREADMDAOLD_H

#include "mda.h"
#include <QString>
#include <QObject>

class DiskReadMdaOldPrivate;

//changed ints to longs on 3/10/16

class DiskReadMdaOld : public QObject {
    Q_OBJECT
public:
    friend class DiskReadMdaOldPrivate;
    explicit DiskReadMdaOld(const QString& path = "");
    DiskReadMdaOld(const DiskReadMdaOld& other);
    DiskReadMdaOld(const Mda& X);
    void operator=(const DiskReadMdaOld& other);
    ~DiskReadMdaOld();

    void setPath(const QString& path);

    long N1() const;
    long N2() const;
    long N3() const;
    long N4() const;
    long N5() const;
    long N6() const;
    long totalSize() const;
    long size(long dim) const;
    double value(long i1, long i2);
    double value(long i1, long i2, long i3, long i4 = 0, long i5 = 0, long i6 = 0);
    double value1(long ii);
    void reshape(long N1, long N2, long N3 = 1, long N4 = 1, long N5 = 1, long N6 = 1);
    void write(const QString& path);

private:
    DiskReadMdaOldPrivate* d;
};

#endif // DISKREADMDAOLD_H
