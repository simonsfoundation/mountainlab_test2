/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/10/2016
*******************************************************/

#include "geom2adj_processor.h"
#include "mda.h"
#include "extract_raw_processor.h" //for str_to_intlist()

class geom2adj_ProcessorPrivate {
public:
    geom2adj_Processor* q;
};

geom2adj_Processor::geom2adj_Processor()
{
    d = new geom2adj_ProcessorPrivate;
    d->q = this;

    this->setName("geom2adj");
    this->setVersion("0.16");
    this->setInputFileParameters("input");
    this->setOutputFileParameters("output");
    this->setRequiredParameters("radius");
    this->setOptionalParameters("channels");
}

geom2adj_Processor::~geom2adj_Processor()
{
    delete d;
}

bool geom2adj_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool geom2adj_Processor::run(const QMap<QString, QVariant>& params)
{
    QString input = params["input"].toString();
    QString output = params["output"].toString();
    double radius = params["radius"].toDouble();
    QString channels_str = params["channels"].toString();
    QVector<int> channels = str_to_intlist(channels_str);

    Mda X(input);
    Mda Y;
    int N = X.N1(); // note transposed rel to appearance in CSV
    int M = X.N2();

    if (channels.isEmpty()) {
        for (int m = 1; m <= M; m++)
            channels << m;
    }

    Y.allocate(M, M);
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            double distsqr = 0;
            for (int n = 0; n < N; n++) { // loop over coord dimension
                double val = X.value(n, i) - X.value(n, j);
                distsqr += val * val;
            }
            if (distsqr <= radius * radius) {
                Y.setValue(1, i, j);
            }
        }
    }

    int N2 = channels.count();
    Mda Y2(N2, N2);
    for (int i = 0; i < N2; i++) {
        for (int j = 0; j < N2; j++) {
            Y2.setValue(Y.value(channels[j] - 1, channels[i] - 1), j, i);
        }
    }

    return Y2.write32(output);
}

class linear_adjacency_matrix_ProcessorPrivate {
public:
    linear_adjacency_matrix_Processor* q;
};

linear_adjacency_matrix_Processor::linear_adjacency_matrix_Processor()
{
    d = new linear_adjacency_matrix_ProcessorPrivate;
    d->q = this;

    this->setName("linear_adjacency_matrix");
    this->setVersion("0.1");
    this->setInputFileParameters("timeseries");
    this->setOutputFileParameters("output");
    this->setRequiredParameters("radius");
}

linear_adjacency_matrix_Processor::~linear_adjacency_matrix_Processor()
{
    delete d;
}

bool linear_adjacency_matrix_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool linear_adjacency_matrix_Processor::run(const QMap<QString, QVariant>& params)
{
    QString timeseries = params["timeseries"].toString();
    QString output = params["output"].toString();
    double radius = params["radius"].toDouble();

    Mda X(timeseries);
    int M = X.N1();

    Mda Y(M, M);
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            if (qAbs(i - j) <= radius) {
                Y.setValue(1, i, j);
            }
        }
    }

    return Y.write32(output);
}
