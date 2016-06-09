/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/4/2016
*******************************************************/

#include "mv_subfirings_processor.h"

#include <QString>
#include <diskreadmda.h>
#include "msmisc.h"

class mv_subfirings_ProcessorPrivate {
public:
    mv_subfirings_Processor* q;
};

mv_subfirings_Processor::mv_subfirings_Processor()
{
    d = new mv_subfirings_ProcessorPrivate;
    d->q = this;

    this->setName("mv_subfirings");
    this->setVersion("0.11");
    this->setInputFileParameters("firings");
    this->setOutputFileParameters("firings_out");
    this->setRequiredParameters("labels");
    this->setOptionalParameters("max_per_label");
}

mv_subfirings_Processor::~mv_subfirings_Processor()
{
    delete d;
}

bool mv_subfirings_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

QList<bool> evenly_distributed_to_use(long num,long num_to_use) {
    QList<bool> ret;
    for (long i=0; i<num; i++) {
        ret << false;
    }
    double stride=num*1.0/num_to_use; //will be greater than 1
    double j=0;
    for (long i=0; i<num_to_use; i++) {
        ret[(long)j]=true;
        j+=stride;
    }
    return ret;
}

bool mv_subfirings(QString firings_path, QString firings_out_path, QList<int> labels, int max_per_label)
{
    QMap<int, long> counts;
    DiskReadMda F(firings_path);
    QSet<int> set;
    foreach (int label, labels) {
        set.insert(label);
    }
    QList<long> inds;
    QList<bool> to_use;
    for (long j = 0; j < F.N2(); j++) {
        int label = (int)F.value(2, j);
        if (set.contains(label)) {
            inds << j;
            to_use << true;
            counts[label]++;
        }
    }

    if (max_per_label) {
        int K = compute_max(labels);
        for (int k = 0; k <= K; k++) {
            if (counts[k] > max_per_label) {
                QList<bool> to_use_k = evenly_distributed_to_use(counts[k], max_per_label);
                long jj = 0;
                for (long i = 0; i < inds.count(); i++) {
                    int label = (int)F.value(2, inds[i]);
                    if (label == k) {
                        to_use[i] = to_use_k[jj];
                        jj++;
                    }
                }
            }
        }
    }

    QList<long> inds2;
    for (long i = 0; i < inds.count(); i++) {
        if (to_use[i])
            inds2 << inds[i];
    }

    Mda out(F.N1(), inds2.count());
    for (long i = 0; i < inds2.count(); i++) {
        for (int j = 0; j < F.N1(); j++) {
            out.setValue(F.value(j, inds2[i]), j, i);
        }
    }
    return out.write64(firings_out_path);
}

bool mv_subfirings_Processor::run(const QMap<QString, QVariant>& params)
{
    QString firings_path = params["firings"].toString();
    QString firings_out_path = params["firings_out"].toString();
    QStringList labels0 = params["labels"].toString().trimmed().split(",");
    QList<int> labels;
    foreach (QString label, labels0) {
        labels << label.trimmed().toInt();
    }
    int max_per_label = params.value("max_per_label", 0).toInt();
    return mv_subfirings(firings_path, firings_out_path, labels, max_per_label);
}
