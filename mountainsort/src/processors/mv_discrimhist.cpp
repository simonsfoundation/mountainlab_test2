#include "mv_discrimhist.h"

#include "diskreadmda.h"

struct disrimhist_data {
    int k1,k2;
    QVector<double> data;
};

/// TODO parallelize mv_distrimhist

QVector<double> get_discrimhist_data(const DiskReadMda &timeseries, const DiskReadMda &firings,int k1,int k2);
bool mv_discrimhist(QString timeseries_path, QString firings_path, QString output_path, mv_discrimhist_opts opts)
{
    DiskReadMda timeseries(timeseries_path);
    DiskReadMda firings(firings_path);

    QList<disrimhist_data> datas;
    for (int i1=0; i1<opts.clusters.count(); i1++) {
        for (int i2=i1+1; i2<opts.clusters.count(); i2++) {
            int k1=opts.clusters[i1];
            int k2=opts.clusters[i2];
            disrimhist_data DD;
            DD.k1=k1;
            DD.k2=k2;
            DD.data=get_discrimhist_data(timeseries,firings,k1,k2);
            datas << DD;
        }
    }

    long total_count=0;
    for (int i=0; i<datas.count(); i++) {
        total_count+=datas[i].data.count();
    }

    Mda output(3,total_count);
    long jj=0;
    for (int i=0; i<datas.count(); i++) {
        int k1=datas[i].k1;
        int k2=datas[i].k2;
        for (long k=0; k<datas[i].data.count(); k++) {
            output.setValue(k1,0,jj);
            output.setValue(k2,1,jj);
            output.setValue(datas[i].data[k],2,jj);
            jj++;
        }
    }

    output.write32(output_path);

    return true;
}

QVector<double> get_discrimhist_data(const DiskReadMda &timeseries, const DiskReadMda &firings,int k1,int k2) {


}
