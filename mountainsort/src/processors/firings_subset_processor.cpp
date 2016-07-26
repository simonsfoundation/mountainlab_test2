/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 7/26/2016
*******************************************************/

#include "firings_subset_processor.h"

#include <diskreadmda.h>
#include <diskwritemda.h>

class firings_subset_ProcessorPrivate
{
public:
    firings_subset_Processor *q;
};

firings_subset_Processor::firings_subset_Processor() {
    d=new firings_subset_ProcessorPrivate;
    d->q=this;

    this->setName("firings_subset");
    this->setVersion("0.1");
    this->setInputFileParameters("firings");
    this->setOutputFileParameters("firings_out");
    this->setRequiredParameters("labels");
    //this->setOptionalParameters();
}

firings_subset_Processor::~firings_subset_Processor() {
    delete d;
}

bool firings_subset_Processor::check(const QMap<QString, QVariant> &params)
{
    if (!this->checkParameters(params)) return false;
    return true;
}

bool firings_subset_Processor::run(const QMap<QString, QVariant> &params)
{
    QString firings=params["firings"].toString();
    QString firings_out=params["firings_out"].toString();
    QStringList labels_str=params["labels"].toString().split(",",QString::SkipEmptyParts);
    QSet<int> labels;
    foreach (QString label,labels_str) {
        labels.insert(label.toInt());
    }

    DiskReadMda X(firings);
    long J=X.N1();
    long L=X.N2();
    QList<long> inds_to_use;
    for (long i=0; i<L; i++) {
        int label0=X.value(2,i);
        if (labels.contains(label0)) {
            inds_to_use << i;
        }
    }

    long L2=inds_to_use.count();
    Mda Y(J,L2);
    for (long i=0; i<L2; i++) {
        for (int j=0; j<J; j++) {
            Y.setValue(X.value(j,inds_to_use[i]),j,i);
        }
    }
    Y.write64(firings_out);

    return true;
}


