/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef MDA32_H
#define MDA32_H

#include "mdabase.h"

class Mda32 : public MdaBase {
public:
    Mda32();
    Mda32(const QString &mda_filename);
    Mda32(long N1,long N2,long N3=1,long N4=1,long N5=1,long N6=1);
    Mda32(const Mda32& other);
    virtual ~Mda32();

    void operator=(const Mda32 &other);

    float get(long i) const {return get32(i);}
    float get(long i1, long i2) const {return get32(i1,i2);}
    float get(long i1, long i2, long i3) const {return get32(i1,i2,i3);}
    float get(long i1, long i2, long i3, long i4, long i5 = 0, long i6 = 0) const {return get32(i1,i2,i3,i4,i5,i6);}

    void set(float val, long i) {set32(val,i);}
    void set(float val, long i1, long i2) {set32(val,i1,i2);}
    void set(float val, long i1, long i2, long i3) {set32(val,i1,i2,i3);}
    void set(float val, long i1, long i2, long i3, long i4, long i5 = 0, long i6 = 0) {set32(val,i1,i2,i3,i4,i5,i6);}

    float *dataPtr() {return dataPtr32();}
    float *dataPtr(long i) {return dataPtr32(i);}
    float *dataPtr(long i1,long i2) {return dataPtr32(i1,i2);}
    float *dataPtr(long i1,long i2,long i3) {return dataPtr32(i1,i2,i3);}
    const float *constDataPtr() const {return constDataPtr32();}
private:
    void copy_from(const Mda32 &other);
};


#endif // MDA_H
