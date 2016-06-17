#include "mountainprocessrunner.h"
#include "mvmisc.h"
#include "taskprogress.h"

DiskReadMda compute_filtered_firings_remotely(QString mlproxy_url, const DiskReadMda& firings, const MVEventFilter& filter)
{
    if (!filter.use_event_filter)
        return firings;

    TaskProgress task(TaskProgress::Calculate, "compute filtered firings");
    MountainProcessRunner X;
    QString processor_name = "mv_firings_filter";
    X.setProcessorName(processor_name);

    QMap<QString, QVariant> params;
    params["firings"] = firings.makePath();
    params["use_shell_split"] = false;
    params["use_event_filter"] = filter.use_event_filter;
    params["min_amplitude"] = 0;
    params["min_detectability_score"] = filter.min_detectability_score;
    params["max_outlier_score"] = filter.max_outlier_score;
    X.setInputParameters(params);
    X.setMLProxyUrl(mlproxy_url);

    QString firings_out_path = X.makeOutputFilePath("firings_out");
    /// TODO get rid of "original_cluster_numbers" and all related functionality
    QString original_cluster_numbers = X.makeOutputFilePath("original_cluster_numbers");
    Q_UNUSED(original_cluster_numbers)

    X.runProcess();
    DiskReadMda ret(firings_out_path);
    return ret;
}

DiskReadMda compute_filtered_firings_locally(const DiskReadMda& firings, const MVEventFilter& filter)
{
    TaskProgress task(TaskProgress::Calculate, "compute filtered firings locally");
    Mda firings0;
    firings.readChunk(firings0, 0, 0, firings.N1(), firings.N2());

    if (filter.use_event_filter) {
        double min_detectablity_score = filter.min_detectability_score;
        double max_outlier_score = filter.max_outlier_score;

        QList<long> inds;
        for (int i = 0; i < firings0.N2(); i++) {
            if (fabs(firings0.value(5, i)) >= min_detectablity_score) {
                if (max_outlier_score) {
                    if (firings0.value(4, i) <= max_outlier_score) {
                        inds << i;
                    }
                } else {
                    inds << i;
                }
            }
        }

        int N2 = inds.count();
        Mda firings_out(firings0.N1(), N2);
        for (int i = 0; i < N2; i++) {
            for (int j = 0; j < firings0.N1(); j++) {
                firings_out.setValue(firings0.value(j, inds[i]), j, i); //speed this up?
            }
        }
        return firings_out;
    } else {
        return firings0;
    }
}
