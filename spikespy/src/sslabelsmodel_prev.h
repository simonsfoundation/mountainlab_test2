#ifndef SSLABELSMODEL
#define SSLABELSMODEL

#include "diskreadmdaold.h"
#include "memorymda_prev.h"

#include <QObject>

class SSLabelsModel {
public:
    virtual ~SSLabelsModel() {}
    //must return a 2xK array of K timepoints along with the K labels
    //these labels represent the K labels that exist between timpoints t1 and t2 inclusive
    virtual MemoryMda getTimepointsLabels(int t1, int t2) = 0;
    virtual qint32 getMaxTimepoint() = 0;
    virtual qint32 getMaxLabel() = 0;
};

#endif // SSLABELSMODEL
