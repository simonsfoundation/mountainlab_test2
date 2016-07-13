#ifndef MEMORYMDA_H
#define MEMORYMDA_H

#include <QObject>

class MemoryMdaPrivate;

class MemoryMda : public QObject {
    Q_OBJECT
public:
    friend class MemoryMdaPrivate;
    explicit MemoryMda(QObject* parent = 0);
    MemoryMda(const MemoryMda& other);
    void operator=(const MemoryMda& other);
    ~MemoryMda();
    void allocate(int N1, int N2, int N3 = 1, int N4 = 1, int N5 = 1, int N6 = 1);
    int size(int dim) const;
    int N1() const;
    int N2() const;
    int N3() const;
    int N4() const;
    int N5() const;
    int N6() const;

    void write(const QString& path);

    double value(int i1, int i2, int i3 = 0, int i4 = 0, int i5 = 0, int i6 = 0);
    void setValue(double val, int i1, int i2, int i3 = 0, int i4 = 0, int i5 = 0, int i6 = 0);

private:
    MemoryMdaPrivate* d;

signals:

public slots:
};

#endif // MEMORYMDA_H
