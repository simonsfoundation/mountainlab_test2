/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 3/31/2016
*******************************************************/

#include "mvhistogramgrid.h"
#include "histogramview.h"
#include "mvutils.h"
#include "taskprogress.h"

#include <QAction>
#include <QGridLayout>
#include <QKeyEvent>
#include <QList>
#include <QPainter>
#include <math.h>
#include "msmisc.h"
#include "mvmisc.h"

class MVHistogramGridPrivate {
public:
    MVHistogramGrid* q;

    QGridLayout* m_grid_layout;
    QList<HistogramView*> m_histogram_views;
    int m_num_columns;
    HorizontalScaleAxisData m_horizontal_scale_axis_data;

    QList<QWidget*> m_child_widgets;

    void do_highlighting();
    int find_view_index_for_k(int k);
    void shift_select_clusters_between(int kA, int kB);
};

class HorizontalScaleAxis : public QWidget {
public:
    HorizontalScaleAxis();
    HorizontalScaleAxisData m_data;

protected:
    void paintEvent(QPaintEvent* evt);
};

MVHistogramGrid::MVHistogramGrid(MVContext* context)
    : MVAbstractView(context)
{
    d = new MVHistogramGridPrivate;
    d->q = this;
    d->m_num_columns = -1;

    QObject::connect(context, SIGNAL(clusterAttributesChanged(int)), this, SLOT(slot_cluster_attributes_changed(int)));
    QObject::connect(context, SIGNAL(currentClusterChanged()), this, SLOT(slot_update_highlighting()));
    QObject::connect(context, SIGNAL(selectedClustersChanged()), this, SLOT(slot_update_highlighting()));

    QGridLayout* GL = new QGridLayout;
    GL->setHorizontalSpacing(12);
    GL->setVerticalSpacing(0);
    GL->setMargin(0);
    this->setLayout(GL);
    d->m_grid_layout = GL;

    {
        QAction* a = new QAction("Export image", this);
        this->addAction(a);
        connect(a, SIGNAL(triggered(bool)), this, SLOT(slot_export_image()));
    }
}

MVHistogramGrid::~MVHistogramGrid()
{
    delete d;
}

QImage MVHistogramGrid::renderImage(int W, int H)
{
    if (!W)
        W = 1800;
    if (!H)
        H = 900;
    int max_row = 0, max_col = 0;
    for (int i = 0; i < d->m_histogram_views.count(); i++) {
        HistogramView* HV = d->m_histogram_views[i];
        int row = HV->property("row").toInt();
        int col = HV->property("col").toInt();
        if (row > max_row)
            max_row = row;
        if (col > max_col)
            max_col = col;
    }
    int NR = max_row + 1, NC = max_col + 1;
    int spacingx = 10;
    int spacingy = 10;
    int W0 = (W - spacingx * (NC + 1)) / NC;
    int H0 = (H - spacingy * (NR + 1)) / NR;

    QImage ret = QImage(W, H, QImage::Format_RGB32);
    QPainter painter(&ret);
    painter.fillRect(0, 0, ret.width(), ret.height(), Qt::white);

    for (int i = 0; i < d->m_histogram_views.count(); i++) {
        HistogramView* HV = d->m_histogram_views[i];
        int row = HV->property("row").toInt();
        int col = HV->property("col").toInt();
        QImage img = HV->renderImage(W0, H0);
        int x0 = spacingx + (W0 + spacingx) * col;
        int y0 = spacingy + (H0 + spacingy) * row;
        painter.drawImage(x0, y0, img);
    }

    return ret;
}

void MVHistogramGrid::paintEvent(QPaintEvent* evt)
{
    QWidget::paintEvent(evt);

    QPainter painter(this);
    if (isCalculating()) {
        //show that something is computing
        painter.fillRect(QRectF(0, 0, width(), height()), mvContext()->color("calculation-in-progress"));
    }
}

void MVHistogramGrid::keyPressEvent(QKeyEvent* evt)
{
    if ((evt->key() == Qt::Key_A) && (evt->modifiers() & Qt::ControlModifier)) {
        QList<int> ks;
        for (int i = 0; i < d->m_histogram_views.count(); i++) {
            ks << d->m_histogram_views[i]->property("k").toInt();
        }
        mvContext()->setSelectedClusters(ks);
    }
    else {
        QWidget::keyPressEvent(evt);
    }
}

void MVHistogramGrid::setHorizontalScaleAxis(HorizontalScaleAxisData X)
{
    d->m_horizontal_scale_axis_data = X;
}

void MVHistogramGrid::setHistogramViews(const QList<HistogramView*> views)
{

    qDeleteAll(d->m_child_widgets);
    qDeleteAll(d->m_histogram_views);
    d->m_child_widgets.clear();
    d->m_histogram_views.clear();
    d->m_histogram_views = views;

    int NUM = views.count();
    int num_rows = (int)sqrt(NUM);
    if (num_rows < 1)
        num_rows = 1;
    int num_cols = (NUM + num_rows - 1) / num_rows;
    d->m_num_columns = num_cols;

    QGridLayout* GL = d->m_grid_layout;
    for (int jj = 0; jj < views.count(); jj++) {
        HistogramView* HV = views[jj];
        int row0 = (jj) / num_cols;
        int col0 = (jj) % num_cols;
        GL->addWidget(HV, row0, col0);
        HV->setProperty("row", row0);
        HV->setProperty("col", col0);
        HV->setProperty("view_index", jj);
        connect(HV, SIGNAL(clicked(Qt::KeyboardModifiers)), this, SLOT(slot_histogram_view_clicked(Qt::KeyboardModifiers)));
        //connect(HV, SIGNAL(rightClicked(Qt::KeyboardModifiers)), this, SLOT(slot_histogram_view_right_clicked(Qt::KeyboardModifiers)));
        connect(HV, SIGNAL(rightClicked(Qt::KeyboardModifiers)), this, SLOT(slot_context_menu()));
        //connect(HV, SIGNAL(activated()), this, SLOT(slot_histogram_view_activated()));
        connect(HV, SIGNAL(activated()), this, SLOT(slot_context_menu()));
        connect(HV, SIGNAL(signalExportHistogramMatrixImage()), this, SLOT(slot_export_image()));
    }

    if (d->m_horizontal_scale_axis_data.use_it) {
        HorizontalScaleAxis* HSA = new HorizontalScaleAxis;
        HSA->m_data = d->m_horizontal_scale_axis_data;
        d->m_child_widgets << HSA;
        GL->addWidget(HSA, num_rows, 0);
    }

    d->do_highlighting();
}

QList<HistogramView*> MVHistogramGrid::histogramViews()
{
    return d->m_histogram_views;
}

void MVHistogramGrid::slot_histogram_view_clicked(Qt::KeyboardModifiers modifiers)
{
    int k = sender()->property("k").toInt();

    if (modifiers & Qt::ControlModifier) {
        mvContext()->clickCluster(k, Qt::ControlModifier);
    }
    else if (modifiers & Qt::ShiftModifier) {
        int k0 = mvContext()->currentCluster();
        d->shift_select_clusters_between(k0, k);
    }
    else {
        mvContext()->clickCluster(k, Qt::NoModifier);
    }
}

void MVHistogramGrid::slot_export_image()
{
    QImage img = this->renderImage();
    user_save_image(img);
}

void MVHistogramGrid::slot_cluster_attributes_changed(int cluster_number)
{
    Q_UNUSED(cluster_number)
    // not implemented for now
}

void MVHistogramGrid::slot_update_highlighting()
{
    d->do_highlighting();
}

void MVHistogramGrid::slot_context_menu()
{
    //this seems a bit circular
    QPoint pt = this->mapFromGlobal(QCursor::pos());

    QMimeData md;
    {
        QByteArray ba;
        QDataStream ds(&ba, QIODevice::WriteOnly);
        ds << mvContext()->selectedClusters();
        md.setData("application/x-mv-cluster", ba); // selected cluster data
    }
    {
        QByteArray ba;
        QDataStream ds(&ba, QIODevice::WriteOnly);
        ds << (quintptr) this;
        md.setData("application/x-mv-view", ba); // this view
    }
    this->requestContextMenu(md, pt);
}

void MVHistogramGridPrivate::do_highlighting()
{
    QList<int> selected_clusters = q->mvContext()->selectedClusters();
    for (int i = 0; i < m_histogram_views.count(); i++) {
        HistogramView* HV = m_histogram_views[i];
        int k = HV->property("k").toInt();
        if (k == q->mvContext()->currentCluster()) {
            HV->setCurrent(true);
        }
        else {
            HV->setCurrent(false);
        }
        if (selected_clusters.contains(k)) {
            HV->setSelected(true);
        }
        else {
            HV->setSelected(false);
        }
    }
}

void MVHistogramGridPrivate::shift_select_clusters_between(int kA, int kB)
{
    QSet<int> selected_clusters = q->mvContext()->selectedClusters().toSet();
    int ind1 = find_view_index_for_k(kA);
    int ind2 = find_view_index_for_k(kB);
    if ((ind1 >= 0) && (ind2 >= 0)) {
        for (int ii = qMin(ind1, ind2); ii <= qMax(ind1, ind2); ii++) {
            selected_clusters.insert(m_histogram_views[ii]->property("k2").toInt());
        }
    }
    else if (ind1 >= 0) {
        selected_clusters.insert(m_histogram_views[ind1]->property("k2").toInt());
    }
    else if (ind2 >= 0) {
        selected_clusters.insert(m_histogram_views[ind2]->property("k2").toInt());
    }
    q->mvContext()->setSelectedClusters(QList<int>::fromSet(selected_clusters));
}

int MVHistogramGridPrivate::find_view_index_for_k(int k)
{
    for (int i = 0; i < m_histogram_views.count(); i++) {
        if (m_histogram_views[i]->property("k").toInt() == k)
            return i;
    }
    return -1;
}

HorizontalScaleAxis::HorizontalScaleAxis()
{
    setFixedHeight(25);
}

void HorizontalScaleAxis::paintEvent(QPaintEvent* evt)
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
    QPointF pt1(W0 / 2, H1);
    QPointF pt2(W0 - margin1, H1);
    QPointF pt3(W0 / 2, H1 - len1);
    QPointF pt4(W0 - margin1, H1 - len1);
    painter.drawLine(pt1, pt2);
    painter.drawLine(pt1, pt3);
    painter.drawLine(pt2, pt4);

    QFont font = painter.font();
    font.setPixelSize(12);
    painter.setFont(font);
    QRect text_box(W0 / 2, H1 + 3, W0 / 2, H1 + 3);
    QString txt = QString("%1").arg(m_data.label);
    painter.drawText(text_box, txt, Qt::AlignCenter | Qt::AlignTop);
}
