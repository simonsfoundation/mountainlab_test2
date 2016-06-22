#include "histogramview.h"
#include <QPaintEvent>
#include <QPainter>
#include <QImage>
#include "mvutils.h"
#include <QMenu>

class HistogramViewPrivate {
public:
    HistogramView* q;
    int m_N;
    float* m_data;
    float* m_bin_centers;
    int* m_bin_counts;
    int m_max_bin_count;
    bool m_update_required;
    int m_num_bins;
    QColor m_fill_color;
    QColor m_line_color;
    int m_hovered_bin_index;
    int m_margin_left, m_margin_right, m_margin_top, m_margin_bottom;
    QString m_title;
    QMap<QString, QColor> m_colors;
    bool m_hovered;
    bool m_current;
    bool m_selected;
    bool m_draw_vertical_axis_at_zero = false;
    MVRange m_xrange = MVRange(0, 0);

    void update_bin_counts();
    QPointF coord2pix(QPointF pt, int W = 0, int H = 0);
    QPointF pix2coord(QPointF pt, int W = 0, int H = 0);
    int get_bin_index_at(QPointF pt);
    void export_image();
    void do_paint(QPainter& painter, int W, int H);
};

HistogramView::HistogramView(QWidget* parent)
    : QWidget(parent)
{
    d = new HistogramViewPrivate;
    d->q = this;
    d->m_N = 0;
    d->m_data = 0;
    d->m_bin_centers = 0;
    d->m_bin_counts = 0;
    d->m_max_bin_count = 0;
    d->m_num_bins = 0;
    d->m_hovered_bin_index = -1;
    d->m_margin_left = d->m_margin_right = d->m_margin_top = d->m_margin_bottom = 5;
    d->m_hovered = false;
    d->m_current = false;
    d->m_selected = false;

    d->m_fill_color = QColor(100, 100, 150);
    d->m_line_color = QColor(150, 150, 150);

    this->setMouseTracking(true);
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slot_context_menu(QPoint)));
}

HistogramView::~HistogramView()
{
    if (d->m_data)
        free(d->m_data);
    if (d->m_bin_counts)
        free(d->m_bin_counts);
    if (d->m_bin_centers)
        free(d->m_bin_centers);
    delete d;
}

void HistogramView::setData(const QList<float> values)
{
    float* tmp = (float*)malloc(sizeof(float) * values.count());
    for (int i = 0; i < values.count(); i++) {
        tmp[i] = values[i];
    }
    setData(values.count(), tmp);
    free(tmp);
}

void HistogramView::setData(int N, float* values)
{
    if (d->m_data)
        free(d->m_data);
    d->m_data = (float*)malloc(sizeof(float) * N);
    for (int i = 0; i < N; i++)
        d->m_data[i] = values[i];
    d->m_N = N;
    d->m_update_required = true;
}

void HistogramView::setBins(float bin_min, float bin_max, int num_bins)
{
    if (num_bins <= 1)
        return;
    if (d->m_bin_centers)
        free(d->m_bin_centers);
    if (d->m_bin_counts)
        free(d->m_bin_counts);
    d->m_bin_centers = (float*)malloc(sizeof(float) * num_bins);
    d->m_bin_counts = (int*)malloc(sizeof(int) * num_bins);
    d->m_num_bins = num_bins;
    for (int i = 0; i < num_bins; i++) {
        d->m_bin_centers[i] = bin_min + (bin_max - bin_min) * i * 1.0F / (num_bins - 1);
    }
    d->m_update_required = true;
}

void HistogramView::autoSetBins(int num_bins)
{
    if (!d->m_data)
        return;
    if (d->m_N == 0)
        return;
    float data_min = d->m_data[0], data_max = d->m_data[1];
    for (int i = 0; i < d->m_N; i++) {
        if (d->m_data[i] < data_min)
            data_min = d->m_data[i];
        if (d->m_data[i] > data_max)
            data_max = d->m_data[i];
    }
    if (data_max == data_min) {
        data_min -= 1;
        data_max += 1;
    }
    float bin_min = data_min;
    float bin_max = data_max;
    setBins(bin_min, bin_max, num_bins);
}

void HistogramView::setFillColor(const QColor& col)
{
    d->m_fill_color = col;
}

void HistogramView::setLineColor(const QColor& col)
{
    d->m_line_color = col;
}

void HistogramView::setTitle(const QString& title)
{
    d->m_title = title;
    update();
}

void HistogramView::setColors(const QMap<QString, QColor>& colors)
{
    d->m_colors = colors;
}

MVRange HistogramView::xRange() const
{
    return d->m_xrange;
}

void HistogramView::setXRange(MVRange range)
{
    if (range == d->m_xrange)
        return;
    d->m_xrange = range;
    update();
}

void HistogramView::setDrawVerticalAxisAtZero(bool val)
{
    if (d->m_draw_vertical_axis_at_zero == val)
        return;
    d->m_draw_vertical_axis_at_zero = val;
    update();
}

void HistogramView::setCurrent(bool val)
{
    if (d->m_current != val) {
        d->m_current = val;
        this->update();
    }
}
void HistogramView::setSelected(bool val)
{
    if (d->m_selected != val) {
        d->m_selected = val;
        this->update();
    }
}

QImage HistogramView::renderImage(int W, int H)
{
    QImage ret = QImage(W, H, QImage::Format_RGB32);
    QPainter painter(&ret);

    bool selected = d->m_selected;
    bool hovered = d->m_hovered;
    bool current = d->m_current;
    int hovered_bin_index = d->m_hovered_bin_index;

    d->m_selected = false;
    d->m_hovered = false;
    d->m_hovered_bin_index = -1;
    d->m_current = false;

    d->do_paint(painter, W, H);

    d->m_selected = selected;
    d->m_hovered = hovered;
    d->m_hovered_bin_index = hovered_bin_index;
    d->m_current = current;

    return ret;
}

QRectF make_rect(QPointF p1, QPointF p2)
{
    float x = qMin(p1.x(), p2.x());
    float y = qMin(p1.y(), p2.y());
    float w = qAbs(p2.x() - p1.x());
    float h = qAbs(p2.y() - p1.y());
    return QRectF(x, y, w, h);
}

QColor lighten(QColor col, int dr, int dg, int db)
{
    int r = col.red() + dr;
    if (r > 255)
        r = 255;
    if (r < 0)
        r = 0;
    int g = col.green() + dg;
    if (g > 255)
        g = 255;
    if (g < 0)
        g = 0;
    int b = col.blue() + db;
    if (b > 255)
        b = 255;
    if (b < 0)
        b = 0;
    return QColor(r, g, b);
}

void HistogramView::paintEvent(QPaintEvent* evt)
{
    Q_UNUSED(evt)
    QPainter painter(this);

    d->do_paint(painter, width(), height());
}

void HistogramView::mousePressEvent(QMouseEvent* evt)
{
    Q_UNUSED(evt);
    emit clicked(evt->modifiers());
    /*
    if ((evt->modifiers() & Qt::ControlModifier) || (evt->modifiers() & Qt::ShiftModifier)) {
        emit control_clicked();
    }
    else {
        emit clicked();
    }
    */
}

void HistogramView::mouseMoveEvent(QMouseEvent* evt)
{
    QPointF pt1 = evt->pos();
    int bin_index = d->get_bin_index_at(pt1);
    if (d->m_hovered_bin_index != bin_index) {
        d->m_hovered_bin_index = bin_index;
        this->update();
    }
}

void HistogramView::enterEvent(QEvent* evt)
{
    Q_UNUSED(evt)
    d->m_hovered = true;
    update();
}

void HistogramView::leaveEvent(QEvent* evt)
{
    Q_UNUSED(evt)
    d->m_hovered = false;
    update();
}

void HistogramView::mouseDoubleClickEvent(QMouseEvent* evt)
{
    Q_UNUSED(evt)
    emit activated();
}

void HistogramView::slot_context_menu(const QPoint& pos)
{
    QMenu M;
    QAction* export_image = M.addAction("Export Histogram Image");
    QAction* export_matrix_image = M.addAction("Export Histogram Matrix Image");
    QAction* selected = M.exec(this->mapToGlobal(pos));
    if (selected == export_image) {
        d->export_image();
    } else if (selected == export_matrix_image) {
        emit this->signalExportHistogramMatrixImage();
    }
}

void HistogramViewPrivate::update_bin_counts()
{
    for (int i = 0; i < m_num_bins; i++) {
        m_bin_counts[i] = 0;
    }
    m_max_bin_count = 0;
    QList<float> list;
    for (int i = 0; i < m_N; i++) {
        list << m_data[i];
    }
    qSort(list);
    if (m_num_bins < 1)
        return;
    float spacing = m_bin_centers[1] - m_bin_centers[0];
    int jj = 0;
    for (int i = 0; i < list.count(); i++) {
        float val = list[i];
        while ((jj + 1 < m_num_bins) && (m_bin_centers[jj] + spacing / 2 < val)) {
            jj++;
        }
        if ((val >= m_bin_centers[jj] - spacing / 2) && (val <= m_bin_centers[jj] + spacing / 2)) {
            m_bin_counts[jj]++;
        }
    }
    for (int i = 0; i < m_num_bins; i++) {
        if (m_bin_counts[i] > m_max_bin_count)
            m_max_bin_count = m_bin_counts[i];
    }
}

QPointF HistogramViewPrivate::coord2pix(QPointF pt, int W, int H)
{
    if (!W)
        W = q->width();
    if (!H)
        H = q->height();

    if (m_num_bins <= 1) {
        return QPointF(0, 0);
    }
    if ((!W) || (!H))
        return QPointF(0, 0);
    if (W <= m_margin_left + m_margin_right + 5)
        return QPointF(0, 0);
    if (H <= m_margin_top + m_margin_bottom + 5)
        return QPointF(0, 0);

    float spacing = m_bin_centers[1] - m_bin_centers[0];
    float xmin = m_bin_centers[0] - spacing / 2;
    float xmax = m_bin_centers[m_num_bins - 1] + spacing / 2;
    float ymin = 0;
    float ymax = m_max_bin_count;

    if (m_xrange.min != m_xrange.max) {
        xmin = m_xrange.min;
        xmax = m_xrange.max;
    }

    float xfrac = 0.5;
    if (xmax > xmin)
        xfrac = (pt.x() - xmin) / (xmax - xmin);
    float yfrac = 0.5;
    if (ymax > ymin)
        yfrac = (pt.y() - ymin) / (ymax - ymin);

    float x0 = m_margin_left + xfrac * (W - m_margin_left - m_margin_right);
    float y0 = H - (m_margin_bottom + yfrac * (H - m_margin_top - m_margin_bottom));

    return QPointF(x0, y0);
}

QPointF HistogramViewPrivate::pix2coord(QPointF pt, int W, int H)
{

    if (!W)
        W = q->width();
    if (!H)
        H = q->height();

    if (m_num_bins <= 1) {
        return QPointF(0, 0);
    }
    if ((!W) || (!H))
        return QPointF(0, 0);
    if (W <= m_margin_left + m_margin_right + 5)
        return QPointF(0, 0);
    if (H <= m_margin_top + m_margin_bottom + 5)
        return QPointF(0, 0);

    float spacing = m_bin_centers[1] - m_bin_centers[0];
    float xmin = m_bin_centers[0] - spacing / 2;
    float xmax = m_bin_centers[m_num_bins - 1] + spacing / 2;
    float ymin = 0;
    float ymax = m_max_bin_count;

    if (m_xrange.min != m_xrange.max) {
        xmin = m_xrange.min;
        xmax = m_xrange.max;
    }

    float xfrac = (pt.x() - m_margin_left) / (W - m_margin_left - m_margin_right);
    float yfrac = 1 - (pt.y() - m_margin_top) / (H - m_margin_top - m_margin_bottom);

    float x0 = xmin + xfrac * (xmax - xmin);
    float y0 = ymin + yfrac * (ymax - ymin);

    return QPointF(x0, y0);
}

int HistogramViewPrivate::get_bin_index_at(QPointF pt_pix)
{
    if (m_num_bins <= 1) {
        return -1;
    }
    QPointF pt = pix2coord(pt_pix);
    float spacing = m_bin_centers[1] - m_bin_centers[0];
    for (int i = 0; i < m_num_bins; i++) {
        if ((pt.x() >= m_bin_centers[i] - spacing / 2) && (pt.x() <= m_bin_centers[i] + spacing / 2)) {
            //if ((0<=pt.y())&&(pt.y()<=m_bin_counts[i])) {
            return i;
            //}
        }
    }
    return -1;
}

void HistogramViewPrivate::export_image()
{
    QImage img = q->renderImage(800, 600);
    user_save_image(img);
}

void HistogramViewPrivate::do_paint(QPainter& painter, int W, int H)
{
    //d->m_colors["view_background"]=QColor(245,245,245);
    //d->m_colors["view_background_highlighted"]=QColor(250,220,200);
    //d->m_colors["view_background_hovered"]=QColor(240,245,240);

    //	QColor hover_color=QColor(150,150,150,80);
    //	QColor current_color=QColor(150,200,200,80);
    //	QColor hover_current_color=QColor(170,200,200,80);

    QRect R(0, 0, W, H);

    if (m_current) {
        painter.fillRect(R, m_colors["view_background_highlighted"]);
    } else if (m_selected) {
        painter.fillRect(R, m_colors["view_background_selected"]);
    } else if (m_hovered) {
        painter.fillRect(R, m_colors["view_background_hovered"]);
    } else {
        painter.fillRect(R, m_colors["view_background"]);
    }

    if (m_selected) {
        painter.setPen(QPen(m_colors["view_frame_selected"], 4));
        painter.drawRect(R);
    }

    if (m_update_required) {
        update_bin_counts();
        m_update_required = false;
    }

    if (m_num_bins <= 1)
        return;
    float spacing = m_bin_centers[1] - m_bin_centers[0];
    for (int i = 0; i < m_num_bins; i++) {
        QPointF pt1 = coord2pix(QPointF(m_bin_centers[i] - spacing / 2, 0), W, H);
        QPointF pt2 = coord2pix(QPointF(m_bin_centers[i] + spacing / 2, m_bin_counts[i]), W, H);
        QRectF R = make_rect(pt1, pt2);
        QColor col = m_fill_color;
        if (i == m_hovered_bin_index)
            col = lighten(col, 15, 15, 15);
        painter.fillRect(R, col);
        painter.setPen(m_line_color);
        painter.drawRect(R);
    }

    if (m_draw_vertical_axis_at_zero) {
        QPointF pt0 = coord2pix(QPointF(0, 0));
        QPointF pt1 = coord2pix(QPointF(0, m_max_bin_count));
        QPen pen = painter.pen();
        pen.setColor(Qt::green);
        painter.setPen(pen);
        painter.drawLine(pt0, pt1);
    }

    if (!m_title.isEmpty()) {
        int text_height = 14;
        QRect R(m_margin_left, 5, W - m_margin_left - m_margin_right, text_height);
        QFont font = painter.font();
        font.setFamily("Arial");
        font.setPixelSize(text_height);
        painter.setFont(font);
        painter.setPen(QColor(100, 60, 60));
        painter.drawText(R, m_title, Qt::AlignLeft | Qt::AlignTop);
    }
}
