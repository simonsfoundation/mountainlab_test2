/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef DISKREADMDA_H
#define DISKREADMDA_H

#include "mda.h"

class DiskReadMdaPrivate;
/**
 * \class DiskReadMda
 * @brief Read-only access to a .mda file, especially useful for huge arrays that cannot be practically loaded into memory.
 *
 * See also Mda
 */
class DiskReadMda {
public:
    friend class DiskReadMdaPrivate;
    DiskReadMda(const QString& path = ""); ///Constructor pointing to the .mda file specified by path (file name).
    DiskReadMda(const DiskReadMda& other); ///Copy constructor
    DiskReadMda(const Mda& X); ///Constructor based on an in-memory array. This enables passing an Mda into a function that expects a DiskReadMda.
    DiskReadMda(const QJsonObject& prv_object);
    virtual ~DiskReadMda();
    void operator=(const DiskReadMda& other);

    ///Set the path (file name) of the .mda file to read.
    void setPath(const QString& file_path);
    void setPrvObject(const QJsonObject& prv_object);

    void setRemoteDataType(const QString& dtype);
    void setDownloadChunkSize(long size);
    long downloadChunkSize();

    QString makePath() const; //not capturing the reshaping
    QJsonObject toPrvObject() const;

    ///The dimensions of the array
    long N1() const;
    long N2() const;
    long N3() const;
    long N4() const;
    long N5() const;
    long N6() const;
    long totalSize() const; //product of N1..N6

    bool reshape(long N1b, long N2b, long N3b = 1, long N4b = 1, long N5b = 1, long N6b = 1);
    DiskReadMda reshaped(long N1b, long N2b, long N3b = 1, long N4b = 1, long N5b = 1, long N6b = 1);

    ///Retrieve a chunk of the vectorized data of size 1xN starting at position i
    bool readChunk(Mda& X, long i, long size) const;
    ///Retrieve a chunk of the vectorized data of size N1xN2 starting at position (i1,i2)
    bool readChunk(Mda& X, long i1, long i2, long size1, long size2) const;
    ///Retrieve a chunk of the vectorized data of size N1xN2xN3 starting at position (i1,i2,i3)
    bool readChunk(Mda& X, long i1, long i2, long i3, long size1, long size2, long size3) const;

    ///A slow method to retrieve the value at location i of the vectorized array for example value(3+4*N1())==value(3,4). Consider using readChunk() instead
    double value(long i) const;
    ///A slow method to retrieve the value at location (i1,i2) of the array. Consider using readChunk() instead
    double value(long i1, long i2) const;
    ///A slow method to retrieve the value at location (i1,i2,i3) of the array. Consider using readChunk() instead
    double value(long i1, long i2, long i3) const;

private:
    DiskReadMdaPrivate* d;
};

///Unit test
void diskreadmda_unit_test();

#endif // DISKREADMDA_H
