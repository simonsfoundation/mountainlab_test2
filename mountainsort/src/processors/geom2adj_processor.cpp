/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/10/2016
*******************************************************/

#include "geom2adj_processor.h"
#include "mda.h"
#include "extract_raw_processor.h" //for str_to_intlist()

class geom2adj_ProcessorPrivate
{
public:
    geom2adj_Processor *q;
};

geom2adj_Processor::geom2adj_Processor() {
    d=new geom2adj_ProcessorPrivate;
    d->q=this;

    this->setName("geom2adj");
    this->setVersion("0.16");
    this->setInputFileParameters("input");
    this->setOutputFileParameters("output");
    this->setRequiredParameters("radius");
    this->setOptionalParameters("channels");
}

geom2adj_Processor::~geom2adj_Processor() {
    delete d;
}

bool geom2adj_Processor::check(const QMap<QString, QVariant> &params)
{
    if (!this->checkParameters(params)) return false;
    return true;
}

bool geom2adj_Processor::run(const QMap<QString, QVariant> &params)
{
    QString input=params["input"].toString();
    QString output=params["output"].toString();
    double radius=params["radius"].toDouble();
    QString channels_str = params["channels"].toString();
    QList<int> channels = str_to_intlist(channels_str);

    Mda X(input);
    Mda Y;
    int M=X.N1();
    int N=X.N2();

    if (channels.isEmpty()) {
        for (int m = 1; m <= M; m++)
            channels << m;
    }

    Y.allocate(N,N);
    for (int i=0; i<N; i++) {
        for (int j=0; j<N; j++) {
            double distsqr=0;
            for (int m=0; m<M; m++) {
                double val=X.value(m,i)-X.value(m,j);
                distsqr+=val*val;
            }
            if (distsqr<=radius*radius) {
                Y.setValue(1,i,j);
            }
        }
    }

    int N2=channels.count();
    Mda Y2(N2,N2);
    for (int i=0; i<N2; i++) {
        for (int j=0; j<N2; j++) {
            Y2.setValue(Y.value(channels[j]-1,channels[i]-1),j,i);
        }
    }

    return Y2.write32(output);
}


