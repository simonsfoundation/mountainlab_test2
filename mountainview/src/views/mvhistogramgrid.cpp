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
#include <QCoreApplication>
#include <QGridLayout>
#include <QKeyEvent>
#include <QList>
#include <QPainter>
#include <math.h>
#include "mlcommon.h"
#include "mvmisc.h"
#include <QScrollArea>
#include <QScrollBar>
#include "actionfactory.h"

class HorizontalScaleAxis : public QWidget {
public:
    HorizontalScaleAxis();
    HorizontalScaleAxisData m_data;

protected:
    void paintEvent(QPaintEvent* evt);
};

class MVHistogramGridPrivate {
public:
    MVHistogramGrid* q;

    QScrollArea *m_scroll_area;
    QWidget* m_grid_widget;
    QGridLayout* m_grid_layout;
    QList<HistogramView*> m_histogram_views;
    double m_preferred_hist_width = 0; //zero means zoomed all the way out
    HorizontalScaleAxisData m_horizontal_scale_axis_data;
    HorizontalScaleAxis* m_horizontal_scale_axis;
    bool m_force_square_matrix = false;

    bool m_pair_mode = true;

    void do_highlighting_and_captions();
    int find_view_index_for_k(int k);
    void shift_select_clusters_between(int kA, int kB);
    void setup_grid(int num_cols);
    void on_resize();
    void get_num_rows_cols_and_height_for_preferred_hist_width(int& num_rows, int& num_cols, int& height, double preferred_width);
};

MVHistogramGrid::MVHistogramGrid(MVContext* context)
    : MVAbstractView(context)
{
    d = new MVHistogramGridPrivate;
    d->q = this;

    d->m_horizontal_scale_axis = new HorizontalScaleAxis;

    QObject::connect(context, SIGNAL(clusterAttributesChanged(int)), this, SLOT(slot_cluster_attributes_changed(int)));
    QObject::connect(context, SIGNAL(clusterPairAttributesChanged(ClusterPair)), this, SLOT(slot_cluster_pair_attributes_changed(ClusterPair)));
    QObject::connect(context, SIGNAL(currentClusterChanged()), this, SLOT(slot_update_highlighting()));
    QObject::connect(context, SIGNAL(selectedClustersChanged()), this, SLOT(slot_update_highlighting()));
    QObject::connect(context, SIGNAL(selectedClusterPairsChanged()), this, SLOT(slot_update_highlighting()));

    QHBoxLayout* layout = new QHBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);
    QScrollArea* SA = new QScrollArea;
    SA->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //SA->setWidgetResizable(true);
    layout->addWidget(SA);
    this->setLayout(layout);
    d->m_scroll_area=SA;

    QWidget* GW = new QWidget;
    QGridLayout* GL = new QGridLayout;
    GW->setLayout(GL);
    SA->setWidget(GW);
    d->m_grid_layout = GL;
    d->m_grid_widget = GW;

    GL->setHorizontalSpacing(12);
    GL->setVerticalSpacing(0);
    GL->setMargin(0);

    ActionFactory::addToToolbar(ActionFactory::ActionType::ZoomIn, this, SLOT(slot_zoom_in()));
    ActionFactory::addToToolbar(ActionFactory::ActionType::ZoomOut, this, SLOT(slot_zoom_out()));

    {
        QAction* a = new QAction("Export image", this);
        this->addAction(a);
        connect(a, SIGNAL(triggered(bool)), this, SLOT(slot_export_image()));
    }
}

MVHistogramGrid::~MVHistogramGrid()
{
    this->stopCalculation();
    delete d;
}

void MVHistogramGrid::setPreferredHistogramWidth(int width)
{
    d->m_preferred_hist_width=width;
    d->on_resize();
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
        if (d->m_pair_mode) {
            QSet<ClusterPair> pairs;
            for (int i = 0; i < d->m_histogram_views.count(); i++) {
                int k1 = d->m_histogram_views[i]->property("k1").toInt();
                int k2 = d->m_histogram_views[i]->property("k2").toInt();
                pairs.insert(ClusterPair(k1, k2));
            }
            mvContext()->setSelectedClusterPairs(pairs);
        }
        else {
            QList<int> ks;
            for (int i = 0; i < d->m_histogram_views.count(); i++) {
                int k = d->m_histogram_views[i]->property("k").toInt();
                if (k) {
                    ks << k;
                }
            }
            mvContext()->setSelectedClusters(ks);
        }
    }
    else {
        QWidget::keyPressEvent(evt);
    }
}

void MVHistogramGrid::resizeEvent(QResizeEvent* evt)
{
    MVAbstractView::resizeEvent(evt);
    d->on_resize();
}

void MVHistogramGrid::setHorizontalScaleAxis(HorizontalScaleAxisData X)
{
    d->m_horizontal_scale_axis_data = X;
}

void MVHistogramGrid::setHistogramViews(const QList<HistogramView*> views)
{
    qDeleteAll(d->m_histogram_views);
    d->m_histogram_views.clear();
    d->m_histogram_views = views;

    for (int jj = 0; jj < views.count(); jj++) {
        HistogramView* HV = views[jj];
        connect(HV, SIGNAL(clicked(Qt::KeyboardModifiers)), this, SLOT(slot_histogram_view_clicked(Qt::KeyboardModifiers)));
        //connect(HV, SIGNAL(activated()), this, SLOT(slot_histogram_view_activated()));
        connect(HV, SIGNAL(activated(QPoint)), this, SLOT(slot_context_menu(QPoint)));
        connect(HV, SIGNAL(signalExportHistogramMatrixImage()), this, SLOT(slot_export_image()));
    }
    d->on_resize();

    d->do_highlighting_and_captions();
}

QList<HistogramView*> MVHistogramGrid::histogramViews()
{
    return d->m_histogram_views;
}

QStringList cluster_pair_set_to_string_list(const QSet<ClusterPair>& pairs)
{
    QStringList ret;
    QList<ClusterPair> list = pairs.toList();
    qSort(list);
    foreach (ClusterPair pair, list) {
        ret << pair.toString();
    }
    return ret;
}

void MVHistogramGrid::prepareMimeData(QMimeData& mimeData, const QPoint& pos)
{
    QByteArray ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    if (this->pairMode()) {
        ds << cluster_pair_set_to_string_list(mvContext()->selectedClusterPairs());
        mimeData.setData("application/x-mv-cluster-pairs", ba); // selected cluster pairs data
    }
    else {
        ds << mvContext()->selectedClusters();
        mimeData.setData("application/x-mv-clusters", ba); // selected cluster data
    }
    MVAbstractView::prepareMimeData(mimeData, pos);
}

void MVHistogramGrid::setPairMode(bool val)
{
    d->m_pair_mode = val;
}

bool MVHistogramGrid::pairMode() const
{
    return d->m_pair_mode;
}

void MVHistogramGrid::setForceSquareMatrix(bool val)
{
    d->m_force_square_matrix = val;
    if (val) {
        d->m_scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    }
    else {
        d->m_scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }
    d->on_resize();
}

void MVHistogramGrid::slot_histogram_view_clicked(Qt::KeyboardModifiers modifiers)
{
    if (d->m_pair_mode) {
        int k1 = sender()->property("k1").toInt();
        int k2 = sender()->property("k2").toInt();
        mvContext()->clickClusterPair(ClusterPair(k1, k2), modifiers);
    }
    else {
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
}

void MVHistogramGrid::slot_export_image()
{
    QImage img = this->renderImage();
    user_save_image(img);
}

void MVHistogramGrid::slot_cluster_attributes_changed(int cluster_number)
{
    Q_UNUSED(cluster_number)
    if (!this->pairMode())
        d->do_highlighting_and_captions();
}

void MVHistogramGrid::slot_cluster_pair_attributes_changed(ClusterPair pair)
{
    Q_UNUSED(pair)
    if (this->pairMode())
        d->do_highlighting_and_captions();
}

void MVHistogramGrid::slot_update_highlighting()
{
    d->do_highlighting_and_captions();
}

void MVHistogramGrid::slot_context_menu(const QPoint& pt)
{
    // IMO this code doesn't work correctly as the "activated" signal
    // doesn't "select" the HV thus we're being triggered over a widget
    // that is different than the selected HV

    HistogramView* HV = qobject_cast<HistogramView*>(sender());
    if (!HV)
        return;
    // send yourself a context menu request
    QContextMenuEvent e(QContextMenuEvent::Mouse, mapFromGlobal(HV->mapToGlobal(pt)));
    QCoreApplication::sendEvent(this, &e);
}

void MVHistogramGrid::slot_zoom_in(double factor)
{
    int hscroll_value=0,vscroll_value=0;
    if (d->m_scroll_area->horizontalScrollBar()) hscroll_value=d->m_scroll_area->horizontalScrollBar()->value();
    if (d->m_scroll_area->verticalScrollBar()) vscroll_value=d->m_scroll_area->verticalScrollBar()->value();
    bool done = false;
    double preferred_hist_width = d->m_preferred_hist_width;
    if ((factor>1)&&(preferred_hist_width==0)) {
        preferred_hist_width=1;
    }
    int iteration_count = 0;
    while (!done) {
        int num_rows1, num_cols1, height1, num_rows2, num_cols2, height2;
        d->get_num_rows_cols_and_height_for_preferred_hist_width(num_rows1, num_cols1, height1, preferred_hist_width);
        preferred_hist_width = qMin(1.0e6,qMin(preferred_hist_width+10,preferred_hist_width * factor));
        d->get_num_rows_cols_and_height_for_preferred_hist_width(num_rows2, num_cols2, height2, preferred_hist_width);
        bool something_changed = (height1 != height2);
        if (something_changed)
            done = true;
        iteration_count++;
        if (iteration_count > 1000) {
            preferred_hist_width = d->m_preferred_hist_width;
            if (factor<1) preferred_hist_width=0;
            done = true;
        }
    }
    d->m_preferred_hist_width = preferred_hist_width;
    d->on_resize();
    if (d->m_scroll_area->horizontalScrollBar()) d->m_scroll_area->horizontalScrollBar()->setValue(hscroll_value);
    if (d->m_scroll_area->verticalScrollBar()) d->m_scroll_area->verticalScrollBar()->setValue(vscroll_value);
}

void MVHistogramGrid::slot_zoom_out(double factor)
{
    slot_zoom_in(1 / factor);
}

void MVHistogramGridPrivate::do_highlighting_and_captions()
{
    QList<int> selected_clusters = q->mvContext()->selectedClusters();
    QSet<ClusterPair> selected_cluster_pairs = q->mvContext()->selectedClusterPairs();
    for (int i = 0; i < m_histogram_views.count(); i++) {
        HistogramView* HV = m_histogram_views[i];
        QString title0, caption0;
        if (m_pair_mode) {
            int k1 = HV->property("k1").toInt();
            int k2 = HV->property("k2").toInt();
            if ((k1) && (k2)) {
                HV->setSelected(selected_cluster_pairs.contains(ClusterPair(k1, k2)));
                //HV->setSelected((selected_clusters.contains(k1)) && (selected_clusters.contains(k2)));
            }
            title0 = QString("%1/%2").arg(k1).arg(k2);
            caption0 = q->mvContext()->clusterPairTagsList(ClusterPair(k1, k2)).join(", ");
        }
        else {
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
            title0 = QString("%1").arg(k);
            caption0 = q->mvContext()->clusterTagsList(k).join(", ");
        }
        HV->setTitle(title0);
        HV->setCaption(caption0);
    }
}

void MVHistogramGridPrivate::shift_select_clusters_between(int kA, int kB)
{
    if (!m_pair_mode) {
        /// TODO (low) handle pair mode case
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
}

void MVHistogramGridPrivate::setup_grid(int num_cols)
{
    QGridLayout* GL = m_grid_layout;
    for (int i = GL->count() - 1; i >= 0; i--) {
        GL->takeAt(i);
    }
    int num_rows = 0;
    for (int jj = 0; jj < m_histogram_views.count(); jj++) {
        HistogramView* HV = m_histogram_views[jj];
        int row0 = (jj) / num_cols;
        int col0 = (jj) % num_cols;
        GL->addWidget(HV, row0, col0);
        HV->setProperty("row", row0);
        HV->setProperty("col", col0);
        HV->setProperty("view_index", jj);
        num_rows = qMax(num_rows, row0 + 1);
    }

    if (m_horizontal_scale_axis_data.use_it) {
        HorizontalScaleAxis* HSA = m_horizontal_scale_axis;
        HSA->m_data = m_horizontal_scale_axis_data;
        GL->addWidget(HSA, num_rows, 0);
    }
}

void MVHistogramGridPrivate::on_resize()
{
    int W = q->width();
    int scroll_bar_width = 30; //how to determine?
    m_grid_widget->setFixedWidth(W - scroll_bar_width);

    if (!m_histogram_views.isEmpty()) {
        int num_rows, num_cols, height;
        get_num_rows_cols_and_height_for_preferred_hist_width(num_rows, num_cols, height, m_preferred_hist_width);
        int height_per_row = height / num_rows;
        if (height_per_row > q->height())
            height = num_rows * q->height();
        if (m_preferred_hist_width==0) {
            height=q->height()-5;
        }
        m_grid_widget->setFixedHeight(height);
        if (m_force_square_matrix) {
            double width=height*q->width()/q->height();
            if (m_preferred_hist_width==0)
                width=q->width()-5;
            m_grid_widget->setFixedWidth(width);
        }
        setup_grid(num_cols);
    }
}

void MVHistogramGridPrivate::get_num_rows_cols_and_height_for_preferred_hist_width(int& num_rows, int& num_cols, int& height, double preferred_hist_width)
{
    int W = q->width();
    int H = q->height();
    if (!(W*H)) {
        num_rows=num_cols=1;
        height=0;
        return;
    }
    double preferred_aspect_ratio = 1.618; //golden ratio
    if (m_force_square_matrix) {
        preferred_aspect_ratio = W * 1.0 / H;
    }

    if (m_force_square_matrix) {
        int NUM = m_histogram_views.count();
        num_rows = qMax(1,(int)sqrt(NUM));
        num_cols = qMax(1,(NUM + num_rows - 1) / num_rows);
        double hist_width=preferred_hist_width;
        if (hist_width*num_cols<W) {
            hist_width=W/(num_cols);
        }
        double hist_height = hist_width / preferred_aspect_ratio;
        height = hist_height * num_rows;
    }
    else {
        bool done = false;
        while (!done) {
            if (preferred_hist_width) {
                num_cols = qMax(1.0, W / preferred_hist_width);
            }
            else {
                num_cols = qMax(1,(int)(m_histogram_views.count()/preferred_aspect_ratio +0.5));
            }
            int hist_width = W / num_cols;
            int hist_height = hist_width / preferred_aspect_ratio;
            num_rows = qMax(1, (m_histogram_views.count() + num_cols - 1) / num_cols);
            height = num_rows * hist_height;
            if (height < H - hist_height * 0.5) {
                preferred_hist_width += 1;
            }
            else
                done = true;
        }
    }
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
