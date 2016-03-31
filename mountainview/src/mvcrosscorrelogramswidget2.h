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
    void setFirings(DiskReadMda &F);
    void setSampleRate(double rate);
    void setColors(const QMap<QString, QColor>& colors);

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

