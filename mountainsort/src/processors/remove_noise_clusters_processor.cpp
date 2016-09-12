/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 3/25/2016
*******************************************************/

#include "remove_noise_clusters_processor.h"

#include <diskreadmda.h>
#include "compute_templates_0.h"

class remove_noise_clusters_ProcessorPrivate {
public:
    remove_noise_clusters_Processor* q;
};

remove_noise_clusters_Processor::remove_noise_clusters_Processor()
{
    d = new remove_noise_clusters_ProcessorPrivate;
    d->q = this;

    this->setName("remove_noise_clusters");
    this->setVersion("0.13");
    this->setInputFileParameters("timeseries", "firings");
    this->setOutputFileParameters("firings_out");
    this->setRequiredParameters("peak_threshold");
}

remove_noise_clusters_Processor::~remove_noise_clusters_Processor()
{
    delete d;
}

bool remove_noise_clusters_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

double compute_peak_amplitude(const Mda32& template0);

bool remove_noise_clusters_Processor::run(const QMap<QString, QVariant>& params)
{
    QString timeseries_path = params["timeseries"].toString();
    QString firings_path = params["firings"].toString();
    double peak_threshold = params["peak_threshold"].toDouble();
    QString firings_out_path = params["firings_out"].toString();

    DiskReadMda32 timeseries(timeseries_path);
    Mda firings(firings_path);

    //clip size doesn't really matter since peak should be toward center
    //but we should keep this small so we can also eliminate crazy asymmetric templates
    int clip_size = 10;
    Mda32 templates = compute_templates_0(timeseries, firings, clip_size);

    int M = templates.N1();
    int K = templates.N3();
    QList<int> to_use;
    int num_clusters_to_use = 0;
    for (int k = 1; k <= K; k++) {
        Mda32 template0;
        templates.getChunk(template0, 0, 0, k - 1, M, clip_size, 1);
        double peak_amplitude = compute_peak_amplitude(template0);
        if (peak_amplitude >= peak_threshold) {
            to_use << 1;
            num_clusters_to_use++;
        }
        else
            to_use << 0;
    }
    QList<int> k_map;
    int k0 = 1;
    for (int kk = 1; kk <= K; kk++) {
        if (to_use[kk - 1]) {
            k_map << k0;
            k0++;
        }
        else
            k_map << 0;
    }

    QVector<long> inds_to_use;
    for (long i = 0; i < firings.N2(); i++) {
        int label = firings.value(2, i);
        if (k_map.value(label - 1, 0) > 0) {
            inds_to_use << i;
        }
    }

    Mda firings_out(firings.N1(), inds_to_use.count());
    for (long i = 0; i < inds_to_use.count(); i++) {
        int label = firings.value(2, inds_to_use[i]);
        int label2 = k_map.value(label - 1, 0);
        for (int j = 0; j < firings.N1(); j++) {
            firings_out.setValue(firings.value(j, inds_to_use[i]), j, i);
        }
        firings_out.setValue(label2, 2, i);
    }

    printf("Using %d of %d clusters and %ld of %ld events.\n", num_clusters_to_use, K, firings_out.N2(), firings.N2());

    return firings_out.write64(firings_out_path);
}

double compute_peak_amplitude(const Mda32& template0)
{
    return qMax(qAbs(template0.minimum()), qAbs(template0.maximum()));
}
