/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/4/2016
*******************************************************/

#include "mv_subfirings_processor.h"

#include <QString>
#include <diskreadmda.h>

class mv_subfirings_ProcessorPrivate
{
public:
    mv_subfirings_Processor *q;
};

mv_subfirings_Processor::mv_subfirings_Processor() {
    d=new mv_subfirings_ProcessorPrivate;
    d->q=this;

    this->setName("mv_subfirings");
    this->setVersion("0.1");
    this->setInputFileParameters("firings");
    this->setOutputFileParameters("firings_out");
    this->setRequiredParameters("labels");
}

mv_subfirings_Processor::~mv_subfirings_Processor() {
    delete d;
}

bool mv_subfirings_Processor::check(const QMap<QString, QVariant> &params)
{
    if (!this->checkParameters(params)) return false;
    return true;
}

bool mv_subfirings(QString firings_path,QString firings_out_path,QList<int> labels) {
    DiskReadMda F(firings_path);
    QSet<int> set;
    foreach (int label,labels) {
        set.insert(label);
    }
    QList<long> inds;
    for (long j=0; j<F.N2(); j++) {
        int label=(int)F.value(2,j);
        if (set.contains(label)) {
            inds << j;
        }
    }
    Mda out(F.N1(),inds.count());
    for (long i=0; i<inds.count(); i++) {
        for (int j=0; j<F.N1(); j++) {
            out.setValue(F.value(j,inds[i]),j,i);
        }
    }
    out.write64(firings_out_path);
    return true;
}

bool mv_subfirings_Processor::run(const QMap<QString, QVariant> &params)
{
    QString firings_path=params["firings"].toString();
    QString firings_out_path=params["firings_out"].toString();
    QStringList labels0=params["labels"].toString().trimmed().split(",");
    QList<int> labels;
    foreach (QString label,labels0) {
        labels << label.trimmed().toInt();
    }
    return mv_subfirings(firings_path,firings_out_path,labels);
}


