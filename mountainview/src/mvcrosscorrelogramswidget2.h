/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 3/31/2016
*******************************************************/

#ifndef MVCROSSCORRELOGRAMSWIDGET2_H
#define MVCROSSCORRELOGRAMSWIDGET2_H

#include "diskreadmda.h"
#include "mvviewagent.h"

#include <QWidget>

struct CrossCorrelogramOptions {
    CrossCorrelogramOptions()
    {
        mode = "undefined";
    }

    /// TODO use enum for mode, not string
    QString mode;
    QList<int> ks;
};

class MVCrossCorrelogramsWidget2Private;
class MVCrossCorrelogramsWidget2 : public QWidget {
    Q_OBJECT
public:
    friend class MVCrossCorrelogramsWidget2Private;
    MVCrossCorrelogramsWidget2(MVViewAgent* view_agent);
    virtual ~MVCrossCorrelogramsWidget2();

    void setOptions(CrossCorrelogramOptions opts);
    void setColors(const QMap<QString, QColor>& colors);
    int currentLabel1();
    int currentLabel2();
    void setCurrentLabel1(int k);
    void setCurrentLabel2(int k);
    QList<int> selectedLabels1();
    QList<int> selectedLabels2();
    void setSelectedLabels1(const QList<int>& L);
    void setSelectedLabels2(const QList<int>& L);
    QImage renderImage(int W = 0, int H = 0);
signals:
    void histogramActivated();
private slots:
    void slot_computation_finished();
    void slot_histogram_view_control_clicked();
    void slot_histogram_view_clicked();
    void slot_histogram_view_activated();
    void slot_export_image();
    void slot_cluster_attributes_changed();
    void slot_update_highlighting();
    void slot_recalculate();
    void slot_view_agent_option_changed(QString name);

private:
    MVCrossCorrelogramsWidget2Private* d;
};

#endif // MVCROSSCORRELOGRAMSWIDGET2_H
