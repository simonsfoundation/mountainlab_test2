/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 3/31/2016
*******************************************************/


#ifndef MVCROSSCORRELOGRAMSWIDGET2_H
#define MVCROSSCORRELOGRAMSWIDGET2_H

#include "diskreadmda.h"

#include <QWidget>

class MVCrossCorrelogramsWidget2Private;
class MVCrossCorrelogramsWidget2 : public QWidget
{
    Q_OBJECT
public:
    friend class MVCrossCorrelogramsWidget2Private;
    MVCrossCorrelogramsWidget2();
    virtual ~MVCrossCorrelogramsWidget2();

    void setLabelPairs(const QList<int> &labels1,const QList<int> &labels2,const QList<QString> &text_labels);
    void setFirings(const DiskReadMda &F);
    void setSampleRate(double rate);
    void setMaxDt(int max_dt);
    void setColors(const QMap<QString, QColor>& colors);
    void setCurrentIndex(int index);
    int currentIndex();
    int currentLabel1();
    int currentLabel2();
    void setCurrentLabel1(int k);
    void setCurrentLabel2(int k);
    QList<int> selectedLabels1();
    QList<int> selectedLabels2();
    void setSelectedLabels1(const QList<int> &L);
    void setSelectedLabels2(const QList<int> &L);
    QList<int> selectedIndices();
    void setSelectedIndices(const QList<int> &X);
    QImage renderImage(int W = 0, int H = 0);
signals:
    void currentIndexChanged();
    void selectedIndicesChanged();
    void indexActivated(int ind);
private slots:
    void slot_computation_finished();
    void slot_histogram_view_control_clicked();
    void slot_histogram_view_clicked();
    void slot_histogram_view_activated();
    void slot_export_image();

private:
    MVCrossCorrelogramsWidget2Private *d;
};

#endif // MVCROSSCORRELOGRAMSWIDGET2_H

