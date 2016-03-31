/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 3/31/2016
*******************************************************/

#include "mvcrosscorrelogramswidget2.h"
#include "computationthread.h"
#include "histogramview.h"
#include "set_progress.h"

#include <QGridLayout>
#include <QList>
#include <QPainter>
#include <math.h>

typedef QList<float> FloatList;

class MVCrossCorrelogramsWidget2Computer : public ComputationThread {
public:
    //input
    DiskReadMda firings;
    QList<int> labels1,labels2;

    void compute();

    //output
    QList<FloatList> data;
};

class MVCrossCorrelogramsWidget2Private {
public:
    MVCrossCorrelogramsWidget2 *q;
    DiskReadMda m_firings;
    QList<int> m_labels1,m_labels2;
    MVCrossCorrelogramsWidget2Computer m_computer;
    QList<FloatList> m_data;
    double m_samplerate;
    int m_current_index;

    QGridLayout* m_grid_layout;
    QList<HistogramView*> m_histogram_views;
    int m_num_columns;

    QList<QWidget*> m_child_widgets;
    QStringList m_text_labels;
    QMap<QString, QColor> m_colors;

    void do_highlighting();
};

MVCrossCorrelogramsWidget2::MVCrossCorrelogramsWidget2()
{
    d=new MVCrossCorrelogramsWidget2Private;
    d->q=this;
    d->m_samplerate=0;
    d->m_current_index=-1;
    d->m_num_columns = -1;

    QGridLayout* GL = new QGridLayout;
    GL->setHorizontalSpacing(20);
    GL->setVerticalSpacing(0);
    GL->setMargin(0);
    this->setLayout(GL);
    d->m_grid_layout = GL;

    d->m_colors["background"] = QColor(240, 240, 240);
    d->m_colors["frame1"] = QColor(245, 245, 245);
    d->m_colors["info_text"] = QColor(80, 80, 80);
    d->m_colors["view_background"] = QColor(245, 245, 245);
    d->m_colors["view_background_highlighted"] = QColor(250, 220, 200);
    d->m_colors["view_background_selected"] = QColor(250, 240, 230);
    d->m_colors["view_background_hovered"] = QColor(240, 245, 240);

    this->setFocusPolicy(Qt::StrongFocus);

    connect(&d->m_computer,SIGNAL(computationFinished()),this,SLOT(slot_computation_finished()));
}

MVCrossCorrelogramsWidget2::~MVCrossCorrelogramsWidget2()
{
    d->m_computer.stopComputation();
    delete d;
}

void MVCrossCorrelogramsWidget2::setLabelPairs(const QList<int> &labels1, const QList<int> &labels2, const QList<QString> &text_labels)
{
    d->m_labels1=labels1;
    d->m_labels2=labels2;
    d->m_text_labels=text_labels;
    d->m_computer.startComputation();
}

void MVCrossCorrelogramsWidget2::setColors(const QMap<QString, QColor>& colors)
{
    d->m_colors = colors;
    foreach (HistogramView* V, d->m_histogram_views) {
        V->setColors(d->m_colors);
    }
}

void MVCrossCorrelogramsWidget2::setFirings(DiskReadMda &F)
{
    d->m_firings=F;
    d->m_computer.startComputation();
}

void MVCrossCorrelogramsWidget2::setSampleRate(double rate)
{
    d->m_samplerate=rate;
    d->m_computer.startComputation();
}

class TimeScaleWidget2 : public QWidget {
public:
    TimeScaleWidget2();
    int m_time_width;

protected:
    void paintEvent(QPaintEvent* evt);
};

TimeScaleWidget2::TimeScaleWidget2()
{
    setFixedHeight(25);
    m_time_width = 0;
}

void TimeScaleWidget2::paintEvent(QPaintEvent* evt)
{
    Q_UNUSED(evt)
    QPainter painter(this);
    QPen pen = painter.pen();
    pen.setColor(Qt::black);
    painter.setPen(pen);

    int W0 = width();
    //int H0=height();
    int H1 = 8;
    int margin1 = 6;
    int len1 = 6;
    QPointF pt1(0 + margin1, H1);
    QPointF pt2(W0 - margin1, H1);
    QPointF pt3(0 + margin1, H1 - len1);
    QPointF pt4(W0 - margin1, H1 - len1);
    painter.drawLine(pt1, pt2);
    painter.drawLine(pt1, pt3);
    painter.drawLine(pt2, pt4);

    QFont font = painter.font();
    font.setPixelSize(12);
    painter.setFont(font);
    QRect text_box(0, H1 + 3, W0, H1 + 3);
    QString txt = QString("%1 ms").arg((int)(m_time_width + 0.5));
    painter.drawText(text_box, txt, Qt::AlignCenter | Qt::AlignTop);
}

float compute_max2(const QList<FloatList>& data0)
{
    float ret = 0;
    for (int i = 0; i < data0.count(); i++) {
        QList<float> tmp = data0[i];
        for (int j = 0; j < tmp.count(); j++) {
            if (tmp[j] > ret)
                ret = tmp[j];
        }
    }
    return ret;
}


void MVCrossCorrelogramsWidget2::slot_computation_finished()
{
    d->m_computer.stopComputation(); //because I'm paranoid
    d->m_data=d->m_computer.data;

    QList<FloatList> data0=d->m_data;

    qDeleteAll(d->m_histogram_views);
    d->m_histogram_views.clear();

    qDeleteAll(d->m_child_widgets);
    d->m_child_widgets.clear();

    QGridLayout* GL = d->m_grid_layout;
    float sample_freq = d->m_samplerate;
    float bin_max = compute_max2(data0);
    float bin_min = -bin_max;
    //int num_bins=100;
    int bin_size = 20;
    int num_bins = (bin_max - bin_min) / bin_size;
    if (num_bins < 100)
        num_bins = 100;
    if (num_bins > 2000)
        num_bins = 2000;
    float time_width = (bin_max - bin_min) / sample_freq * 1000;

    int NUM = data0.count() - 1;
    int num_rows = (int)sqrt(NUM);
    if (num_rows < 1)
        num_rows = 1;
    int num_cols = (NUM + num_rows - 1) / num_rows;
    d->m_num_columns = num_cols;
    for (int ii=0; ii<data0.count(); ii++) {
        set_progress("Computing cross correlograms ***","",ii*1.0/d->m_labels1.count());
        HistogramView* HV = new HistogramView;
        HV->setData(data0[ii]);
        HV->setColors(d->m_colors);
        //HV->autoSetBins(50);
        HV->setBins(bin_min, bin_max, num_bins);
        QString title0=d->m_text_labels.value(ii);
        HV->setTitle(title0);
        int row0 = (ii - 1) / num_cols;
        int col0 = (ii - 1) % num_cols;
        GL->addWidget(HV, row0, col0);
        HV->setProperty("row", row0);
        HV->setProperty("col", col0);
        HV->setProperty("index", ii);
        connect(HV, SIGNAL(control_clicked()), this, SLOT(slot_histogram_view_control_clicked()));
        connect(HV, SIGNAL(clicked()), this, SLOT(slot_histogram_view_clicked()));
        connect(HV, SIGNAL(activated()), this, SLOT(slot_histogram_view_activated()));
        connect(HV, SIGNAL(signalExportHistogramMatrixImage()), this, SLOT(slot_export_image()));
        d->m_histogram_views << HV;
    }

    qDebug() << __FILE__ << __LINE__;

    TimeScaleWidget2* TSW = new TimeScaleWidget2;
    TSW->m_time_width = time_width;
    GL->addWidget(TSW, num_rows, 0);

    d->m_child_widgets << TSW;

    set_progress("Loading cross correlograms...","",1);

    qDebug() << __FILE__ << __LINE__;

}

void MVCrossCorrelogramsWidget2::slot_histogram_view_control_clicked()
{

}

void MVCrossCorrelogramsWidget2::slot_histogram_view_clicked()
{

}

void MVCrossCorrelogramsWidget2::slot_histogram_view_activated()
{

}

void MVCrossCorrelogramsWidget2::slot_export_image()
{

}

void MVCrossCorrelogramsWidget2Computer::compute()
{

}
