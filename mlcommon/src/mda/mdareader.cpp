#include "mda/mdareader.h"
#include <QSaveFile>
#include <QFile>
#include <QtEndian>
#include <QFileInfo>

class MdaIOHandler {
public:
    MdaIOHandler()
        : dev(0)
    {
    }
    virtual ~MdaIOHandler() {}
    virtual bool canRead() const = 0;
    virtual bool canWrite() const { return false; }
    virtual bool read(Mda*) = 0;
    virtual bool read(Mda32*) { return false; }
    virtual bool write(const Mda&) { return false; }
    virtual bool write(const Mda32&) { return false; }
    QIODevice* device() const { return dev; }
    void setDevice(QIODevice* d) { dev = d; }
    QByteArray format() const { return fmt; }
    void setFormat(const QByteArray& ba) { fmt = ba; }
private:
    QIODevice* dev;
    QByteArray fmt;
};

class MdaIOHandlerMDA : public MdaIOHandler {
public:
    enum DataType {
        Complex = -1,
        Byte = -2,
        Float32 = -3,
        Int16 = -4,
        Int32 = -5,
        UInt16 = -6,
        Float64 = -7,
        UInt32 = -8
    };

    const int maxDims = 50;
    MdaIOHandlerMDA(QIODevice* d, const QByteArray& fmt)
    {
        setFormat(fmt);
        setDevice(d);
    }
    bool canRead() const
    {
        if (!device())
            return false;
        if (format().toLower() == "mda")
            return true;
        if (format().toLower().startsWith("mda."))
            return true;
        if (format().isEmpty()) {
            // try to auto detect by file name
            if(QFileDevice *f = qobject_cast<QFileDevice*>(device())) {
                QFileInfo finfo(f->fileName());
                if (finfo.suffix().toLower() == "mda")
                    return true;
            }
        }
        return false;
    }

    bool canWrite() const
    {
        if (!device())
            return false;
        if (format().toLower() == "mda")
            return true;
        if (format().toLower().startsWith("mda."))
            return true;
        return false;
    }
    bool read(Mda* mda)
    {
        Header header;
        if (!readLE(&header.data_type))
            return false;
        if (!readLE(&header.num_bytes_per_entry))
            return false;
        if (!readLE(&header.num_dims))
            return false;
        if (header.num_dims <= 0 || header.num_dims > maxDims)
            return false;
        header.dims.resize(header.num_dims);
        for (size_t i = 0; i < (size_t)header.num_dims; ++i) {
            if (!readLE(header.dims.data() + i))
                return false;
        }
        while (header.dims.size() < 6)
            header.dims.append(1);
        if (!mda->allocate(header.dims[0], header.dims[1], header.dims[2], header.dims[3], header.dims[4], header.dims[5]))
            return false;
        switch (header.data_type) {
        case Byte:
            return readData<unsigned char>(mda->dataPtr(), mda->totalSize());
        case Float32:
            return readData<float>(mda->dataPtr(), mda->totalSize());
        case Int16:
            return readData<int16_t>(mda->dataPtr(), mda->totalSize());
        case Int32:
            return readData<int32_t>(mda->dataPtr(), mda->totalSize());
        case UInt16:
            return readData<uint16_t>(mda->dataPtr(), mda->totalSize());
        case Float64:
            return readData<double>(mda->dataPtr(), mda->totalSize());
        case UInt32:
            return readData<uint32_t>(mda->dataPtr(), mda->totalSize());
        default:
            return false;
        }
        return true;
    }
    bool read(Mda32 *mda) {
        Header header;
        if (!readLE(&header.data_type))
            return false;
        if (!readLE(&header.num_bytes_per_entry))
            return false;
        if (!readLE(&header.num_dims))
            return false;
        if (header.num_dims <= 0 || header.num_dims > maxDims)
            return false;
        header.dims.resize(header.num_dims);
        for (size_t i = 0; i < (size_t)header.num_dims; ++i) {
            if (!readLE(header.dims.data() + i))
                return false;
        }
        while (header.dims.size() < 6)
            header.dims.append(1);
        if (!mda->allocate(header.dims[0], header.dims[1], header.dims[2], header.dims[3], header.dims[4], header.dims[5]))
            return false;
        switch (header.data_type) {
        case Byte:
            return readData<unsigned char>(mda->dataPtr(), mda->totalSize());
        case Float32:
            return readData<float>(mda->dataPtr(), mda->totalSize());
        case Int16:
            return readData<int16_t>(mda->dataPtr(), mda->totalSize());
        case Int32:
            return readData<int32_t>(mda->dataPtr(), mda->totalSize());
        case UInt16:
            return readData<uint16_t>(mda->dataPtr(), mda->totalSize());
        case Float64:
            return readData<double>(mda->dataPtr(), mda->totalSize());
        case UInt32:
            return readData<uint32_t>(mda->dataPtr(), mda->totalSize());
        default:
            return false;
        }
        return true;
    }

    bool write(const Mda& mda)
    {
        Header header;
        if (format().toLower().startsWith("mda.")) {
            QByteArray subFormat = format().toLower().mid(4);
            if (subFormat == "byte") {
                header.data_type = Byte;
                header.num_bytes_per_entry = 1;
            } else if (subFormat == "float" || subFormat == "float32") {
                header.data_type = Float32;
                header.num_bytes_per_entry = 4;
            } else if (subFormat == "int16") {
                header.data_type = Int16;
                header.num_bytes_per_entry = 2;
            } else if (subFormat == "int" || subFormat == "int32") {
                header.data_type = Int32;
                header.num_bytes_per_entry = 4;
            } else if (subFormat == "uint16") {
                header.data_type = UInt16;
                header.num_bytes_per_entry = 2;
            } else if (subFormat == "double" || subFormat == "float64") {
                header.data_type = Float64;
                header.num_bytes_per_entry = 8;
            } else if (subFormat == "uint" || subFormat == "uint32") {
                header.data_type = UInt32;
                header.num_bytes_per_entry = 4;
            } else {
                return false;
            }
        } else {
            header.data_type = Float64;
            header.num_bytes_per_entry = 8;
        }
        header.num_dims = mda.ndims();
        header.dims.resize(header.num_dims);
        for (int i = 0; i < header.num_dims; ++i) {
            header.dims[i] = mda.size(i);
        }
        // commit the header
        writeLE(header.data_type);
        writeLE(header.num_bytes_per_entry);
        writeLE(header.num_dims);
        for (int32_t dim : header.dims)
            writeLE(dim);

        // commit data
        switch (header.data_type) {
        case Byte:
            return writeData<unsigned char>(mda.constDataPtr(), mda.totalSize());
        case Float32:
            return writeData<float>(mda.constDataPtr(), mda.totalSize());
        case Int16:
            return writeData<int16_t>(mda.constDataPtr(), mda.totalSize());
        case Int32:
            return writeData<int32_t>(mda.constDataPtr(), mda.totalSize());
        case UInt16:
            return writeData<uint16_t>(mda.constDataPtr(), mda.totalSize());
        case Float64:
            return writeData<double>(mda.constDataPtr(), mda.totalSize());
        case UInt32:
            return writeData<uint32_t>(mda.constDataPtr(), mda.totalSize());
        default:
            return false;
        }
    }

    bool write(const Mda32& mda)
    {
        Header header;
        if (format().toLower().startsWith("mda.")) {
            QByteArray subFormat = format().toLower().mid(4);
            if (subFormat == "byte") {
                header.data_type = Byte;
                header.num_bytes_per_entry = 1;
            } else if (subFormat == "float" || subFormat == "float32") {
                header.data_type = Float32;
                header.num_bytes_per_entry = 4;
            } else if (subFormat == "int16") {
                header.data_type = Int16;
                header.num_bytes_per_entry = 2;
            } else if (subFormat == "int" || subFormat == "int32") {
                header.data_type = Int32;
                header.num_bytes_per_entry = 4;
            } else if (subFormat == "uint16") {
                header.data_type = UInt16;
                header.num_bytes_per_entry = 2;
            } else if (subFormat == "double" || subFormat == "float64") {
                header.data_type = Float64;
                header.num_bytes_per_entry = 8;
            } else if (subFormat == "uint" || subFormat == "uint32") {
                header.data_type = UInt32;
                header.num_bytes_per_entry = 4;
            } else {
                return false;
            }
        } else {
            header.data_type = Float64;
            header.num_bytes_per_entry = 8;
        }
        header.num_dims = mda.ndims();
        header.dims.resize(header.num_dims);
        for (int i = 0; i < header.num_dims; ++i) {
            header.dims[i] = mda.size(i);
        }
        // commit the header
        writeLE(header.data_type);
        writeLE(header.num_bytes_per_entry);
        writeLE(header.num_dims);
        for (int32_t dim : header.dims)
            writeLE(dim);

        // commit data
        switch (header.data_type) {
        case Byte:
            return writeData<unsigned char>(mda.constDataPtr(), mda.totalSize());
        case Float32:
            return writeData<float>(mda.constDataPtr(), mda.totalSize());
        case Int16:
            return writeData<int16_t>(mda.constDataPtr(), mda.totalSize());
        case Int32:
            return writeData<int32_t>(mda.constDataPtr(), mda.totalSize());
        case UInt16:
            return writeData<uint16_t>(mda.constDataPtr(), mda.totalSize());
        case Float64:
            return writeData<double>(mda.constDataPtr(), mda.totalSize());
        case UInt32:
            return writeData<uint32_t>(mda.constDataPtr(), mda.totalSize());
        default:
            return false;
        }
    }

    struct Header {
        int32_t data_type;
        int32_t num_bytes_per_entry;
        int32_t num_dims;
        QVector<int32_t> dims;
    };
    template <typename T>
    bool readLE(T* ptr)
    {
        if (device()->read((char*)ptr, sizeof(T)) != sizeof(T))
            return false;
        *ptr = qFromLittleEndian(*ptr);
        return true;
    }
    template <typename T>
    bool writeLE(T val)
    {
        val = qToLittleEndian(val);
        return (device()->write((char*)&val, sizeof(T)) == sizeof(T));
    }

    template <typename src, typename dst>
    bool readData(dst* ptr, size_t cnt = 1)
    {
        src val;
        for (size_t i = 0; i < cnt; ++i) {
            if (!readLE(&val))
                return false;
            *(ptr++) = val;
        }
        return true;
    }
    template <typename dst, typename src>
    bool writeData(src* ptr, size_t cnt = 1)
    {
        dst val;
        for (size_t i = 0; i < cnt; ++i) {
            val = *(ptr++);
            if (!writeLE(val))
                return false;
        }
        return true;
    }
};

class MdaReaderPrivate {
public:
    MdaReaderPrivate(MdaReader* qq, QIODevice* dev = 0)
        : q(qq)
        , device(dev)
        , ownsDevice(false)
    {
    }

    MdaReader* q;
    QIODevice* device;
    QByteArray format;
    bool ownsDevice;
};

MdaReader::MdaReader()
    : d(new MdaReaderPrivate(this))
{
}

MdaReader::MdaReader(QIODevice* dev, const QByteArray& format)
    : d(new MdaReaderPrivate(this, dev))
{
    d->format = format;
}

MdaReader::MdaReader(const QString& fileName, const QByteArray& format)
    : d(new MdaReaderPrivate(this))
{
    d->format = format;
    d->device = new QFile(fileName);
    d->ownsDevice = true;
}

MdaReader::~MdaReader()
{
    if (d->ownsDevice)
        d->device->deleteLater();
    delete d;
}

bool MdaReader::canRead() const
{
    MdaIOHandlerMDA h(device(), format());
    return h.canRead();
}

QIODevice* MdaReader::device() const
{
    return d->device;
}

QString MdaReader::fileName() const
{
    if (QFile* f = qobject_cast<QFile*>(d->device)) {
        return f->fileName();
    }
    return QString();
}

QByteArray MdaReader::format() const
{
    return d->format;
}

Mda MdaReader::read()
{
    Mda mda;
    if (read(&mda)) {
        return std::move(mda);
    }
    return Mda();
}

Mda32 MdaReader::read32()
{
    Mda32 mda;
    if (read(&mda)) {
        return std::move(mda);
    }
    return Mda32();
}

bool MdaReader::read(Mda* mda)
{
    if (!device()->isOpen() && !device()->open(QIODevice::ReadOnly))
        return false;
    MdaIOHandlerMDA h(device(), format());
    if (!h.canRead()) return false;
    return h.read(mda);
}

bool MdaReader::read(Mda32 *mda)
{
    if (!device()->isOpen() && !device()->open(QIODevice::ReadOnly))
        return false;
    MdaIOHandlerMDA h(device(), format());
    if (!h.canRead()) return false;
    return h.read(mda);
}

void MdaReader::setDevice(QIODevice* dev)
{
    if (d->device && d->ownsDevice) {
        d->device->deleteLater();
        d->device = 0;
        d->ownsDevice = false;
    }
    d->device = dev;
}

void MdaReader::setFileName(const QString& fileName)
{
    if (d->device && d->ownsDevice) {
        d->device->deleteLater();
        d->device = 0;
    }
    d->device = new QFile(fileName);
    d->ownsDevice = true;
}

class MdaWriterPrivate {
public:
    MdaWriterPrivate(MdaWriter* qq, QIODevice* dev = 0)
        : q(qq)
        , device(dev)
        , ownsDevice(false)
    {
    }

    MdaWriter* q;
    QIODevice* device;
    QByteArray format;
    bool ownsDevice;
};

MdaWriter::MdaWriter()
    : d(new MdaWriterPrivate(this))
{
}

MdaWriter::MdaWriter(QIODevice* dev, const QByteArray& format)
    : d(new MdaWriterPrivate(this, dev))
{
    d->format = format;
}

MdaWriter::MdaWriter(const QString& fileName, const QByteArray& format)
    : d(new MdaWriterPrivate(this))
{
    d->device = new QSaveFile(fileName);
    d->ownsDevice = true;
    d->format = format;
}

MdaWriter::~MdaWriter()
{
    if (d->ownsDevice) {
        if (d->device)
            d->device->deleteLater();
    }
    delete d;
}

bool MdaWriter::canWrite() const
{
    MdaIOHandlerMDA h(device(), format());
    return h.canWrite();
}

QIODevice* MdaWriter::device() const
{
    return d->device;
}

QString MdaWriter::fileName() const
{
    if (QFileDevice* f = qobject_cast<QFileDevice*>(d->device))
        return f->fileName();

    return QString();
}

QByteArray MdaWriter::format() const
{
    return d->format;
}

void MdaWriter::setDevice(QIODevice* dev)
{
    if (d->device && d->ownsDevice) {
        d->device->deleteLater();
        d->device = 0;
        d->ownsDevice = false;
    }
    d->device = dev;
}

void MdaWriter::setFileName(const QString& fileName)
{
    if (d->device && d->ownsDevice) {
        d->device->deleteLater();
        d->device = 0;
    }
    d->device = new QSaveFile(fileName);
    d->ownsDevice = true;
}

bool MdaWriter::write(const Mda& mda)
{
    bool closeAfterWrite = !device()->isOpen();
    if (!device()->isOpen() && !device()->open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;
    MdaIOHandlerMDA h(device(), format());
    if (!h.canWrite()) return false;
    bool res = h.write(mda);
    if (QSaveFile *f = qobject_cast<QSaveFile*>(device()))
        f->commit();
    else if (closeAfterWrite)
        device()->close();
    return res;
}

bool MdaWriter::write(const Mda32 &mda)
{
    bool closeAfterWrite = !device()->isOpen();
    if (!device()->isOpen() && !device()->open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;
    MdaIOHandlerMDA h(device(), format());
    if (!h.canWrite()) return false;
    bool res = h.write(mda);
    if (QSaveFile *f = qobject_cast<QSaveFile*>(device()))
        f->commit();
    else if (closeAfterWrite)
        device()->close();
    return res;
}
