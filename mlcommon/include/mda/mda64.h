#ifndef MDA64_H
#define MDA64_H

#include "mdabase.h"

class Mda64 : public MdaBase {
public:
    Mda64();
    Mda64(const QString &mda_filename);
    Mda64(long N1,long N2,long N3=1,long N4=1,long N5=1,long N6=1);
    Mda64(const Mda64& other);
    virtual ~Mda64();

    void operator=(const Mda64 &other);

    double get(long i) const {return get64(i);}
    double get(long i1, long i2) const {return get64(i1,i2);}
    double get(long i1, long i2, long i3) const {return get64(i1,i2,i3);}
    double get(long i1, long i2, long i3, long i4, long i5 = 0, long i6 = 0) const {return get64(i1,i2,i3,i4,i5,i6);}

    void set(double val, long i) {set64(val,i);}
    void set(double val, long i1, long i2) {set64(val,i1,i2);}
    void set(double val, long i1, long i2, long i3) {set64(val,i1,i2,i3);}
    void set(double val, long i1, long i2, long i3, long i4, long i5 = 0, long i6 = 0) {set64(val,i1,i2,i3,i4,i5,i6);}

    double *dataPtr() {return dataPtr64();}
    double *dataPtr(long i) {return dataPtr64(i);}
    double *dataPtr(long i1,long i2) {return dataPtr64(i1,i2);}
    double *dataPtr(long i1,long i2,long i3) {return dataPtr64(i1,i2,i3);}
    const double *constDataPtr() const {return constDataPtr64();}
private:
    void copy_from(const Mda64 &other);
};

#endif // MDA64_H

