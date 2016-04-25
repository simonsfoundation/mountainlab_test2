/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef isosplit2_h
#define isosplit2_h

#include "mda.h"
#include <QList>

QList<int> isosplit2(Mda &X,float isocut_threshold=1.5,int K_init=30,bool verbose=false);
void test_isosplit2_routines();

QList<long> find_inds(const QList<int> &labels,int k);

#endif
