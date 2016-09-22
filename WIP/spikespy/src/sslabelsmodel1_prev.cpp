#include "diskreadmdaold.h"
#include "diskwritemda.h"
#include "memorymda_prev.h"
#include "sslabelsmodel1_prev.h"

class SSLabelsModel1Private {
public:
    SSLabelsModel1* q;
    DiskReadMdaOld* m_timepoints_labels;
    bool m_timepoints_labels_is_owner;
};

SSLabelsModel1::SSLabelsModel1()
{
    d = new SSLabelsModel1Private;
    d->q = this;
    d->m_timepoints_labels = 0;
    d->m_timepoints_labels_is_owner = false;
}

SSLabelsModel1::~SSLabelsModel1()
{
    if (d->m_timepoints_labels)
        if (d->m_timepoints_labels_is_owner)
            delete d->m_timepoints_labels;
}

/*
void SSLabelsModel1::setTimepointsLabels(const Disk &timepoints, const Mda &labels)
{
	if (timepoints.totalSize()!=labels.totalSize()) return;
	d->m_timepoints=timepoints;
	d->m_labels=labels;
}
*/

void SSLabelsModel1::setTimepointsLabels(DiskReadMdaOld* TL, bool is_owner)
{
    d->m_timepoints_labels = TL;
    d->m_timepoints_labels_is_owner = is_owner;
}

MemoryMda SSLabelsModel1::getTimepointsLabels(int t1, int t2)
{
    int K = d->m_timepoints_labels->N2();
    int ct = 0;
    for (int i = 0; i < K; i++) {
        int t0 = (int)d->m_timepoints_labels->value(0, i);
        if ((t1 <= t0) && (t0 <= t2)) {
            ct++;
        }
    }
    MemoryMda ret; //ret.setDataType(MDAIO_TYPE_INT32);
    ret.allocate(2, ct);
    ct = 0;
    for (int i = 0; i < K; i++) {
        int t0 = (int)d->m_timepoints_labels->value(0, i);
        if ((t1 <= t0) && (t0 <= t2)) {
            ret.setValue(t0, 0, ct);
            float val0 = d->m_timepoints_labels->value(1, i);
            ret.setValue(val0, 1, ct);
            ct++;
        }
    }
    return ret;
}

qint32 SSLabelsModel1::getMaxTimepoint()
{
    int K = d->m_timepoints_labels->N2();
    qint32 ret = 0;
    for (int i = 0; i < K; i++) {
        ret = qMax(ret, (qint32)d->m_timepoints_labels->value(0, i));
    }
    return ret;
}

qint32 SSLabelsModel1::getMaxLabel()
{
    int K = d->m_timepoints_labels->N2();
    qint32 ret = 0;
    for (int i = 0; i < K; i++) {
        ret = qMax(ret, (qint32)d->m_timepoints_labels->value(1, i));
    }
    return ret;
}
