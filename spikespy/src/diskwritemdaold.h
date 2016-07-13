#ifndef DISKWRITEMDAOLD_H
#define DISKWRITEMDAOLD_H

#include <QObject>
#include "diskreadmdaold.h"
#include "mdaio.h"

class DiskWriteMdaOldPrivate;

class DiskWriteMdaOld : public QObject {
public:
    friend class DiskWriteMdaOldPrivate;
    DiskWriteMdaOld(const QString& path = "");
    ~DiskWriteMdaOld();

    void setPath(const QString& path);
    void useTemporaryFile();

    void setDataType(int data_type);
    void allocate(int N1, int N2, int N3 = 1, int N4 = 1, int N5 = 1, int N6 = 1);
    void setValue(double val, int i1, int i2, int i3 = 0, int i4 = 0, int i5 = 0, int i6 = 0);
    void setValues(double* vals, int i1, int i2, int i3 = 0, int i4 = 0, int i5 = 0, int i6 = 0);
    DiskReadMdaOld toReadMda();

private:
    DiskWriteMdaOldPrivate* d;
};

#endif // DISKWRITEMDAOLD_H
