#include "histogramview.h"
#include <QPaintEvent>
#include <QPainter>
#include <QImage>
#include "mvutils.h"
#include "mlcommon.h"
#include <QMenu>

class HistogramViewPrivate {
public:
    HistogramView* q;
    QVector<double> m_data;
    QVector<double> m_second_data;
    QVector<double> m_bin_centers;
    QVector<double> m_bin_lefts;
    QVector<double> m_bin_rights;
    QVector<int> m_bin_counts;
    QVector<int> m_second_bin_counts;
    QVector<double> m_bin_densities;
    QVector<double> m_second_bin_densities;
    double m_max_bin_density=0;
    bool m_update_required;
    QColor m_fill_color;
    QColor m_line_color;
    int m_hovered_bin_index;
    int m_margin_left, m_margin_right, m_margin_top, m_margin_bottom;
    QString m_title;
    QString m_caption;
    QMap<QString, QColor> m_colors;
    bool m_hovered;
    bool m_current;
    bool m_selected;
    bool m_draw_vertical_axis_at_zero = false;
    QList<double> m_vertical_lines;
    MVRange m_xrange = MVRange(0, 0);
    bool m_log_mode=false;
    double m_log_timescale=10;

    double transform1(double t); //for example log, or identity
    double transform2(double x); //for example exp, or identity
    QVector<double> transform1(const QVector<double> &t);

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
    d->m_hovered_bin_index = -1;
    d->m_margin_left = d->m_margin_right = d->m_margin_top = 5;
    d->m_margin_bottom = 14;
    d->m_hovered = false;
    d->m_current = false;
    d->m_selected = false;

    d->m_fill_color = QColor(120, 120, 150);
    d->m_line_color = QColor(100, 100, 130);

    this->setMouseTracking(true);
    //this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slot_context_menu(QPoint)));
}

HistogramView::~HistogramView()
{
    delete d;
}

void HistogramView::setData(const QVector<double>& values)
{
    d->m_data=values;
    d->m_update_required=true;
}

void HistogramView::setSecondData(const QVector<double>& values)
{
    d->m_second_data = values;
    d->m_update_required = true;
}

void HistogramView::setBins(const QVector<double> &bin_centers)
{
    d->m_bin_centers=bin_centers;
    d->m_bin_lefts=QVector<double>(bin_centers.count());
    d->m_bin_rights=QVector<double>(bin_centers.count());
    d->m_bin_counts=QVector<int>(bin_centers.count());
    d->m_second_bin_counts=QVector<int>(bin_centers.count());
    d->m_bin_densities=QVector<double>(bin_centers.count());
    d->m_second_bin_densities=QVector<double>(bin_centers.count());
    if (bin_centers.count()<=1) return;
    for (int i=0; i<d->m_bin_centers.count(); i++) {
        if (i==0) d->m_bin_lefts[i]=d->m_bin_centers[i]-(d->m_bin_centers[i+1]-d->m_bin_centers[i]);
        else d->m_bin_lefts[i]=(d->m_bin_centers[i]+d->m_bin_centers[i-1])/2;
        if (i==d->m_bin_centers.count()-1)
            d->m_bin_rights[i]=d->m_bin_centers[i]+(d->m_bin_centers[i]-d->m_bin_centers[i-1]);
        else
            d->m_bin_rights[i]=(d->m_bin_centers[i]+d->m_bin_centers[i+1])/2;
    }
    d->m_update_required = true;
}

void HistogramView::setBins(double bin_min, double bin_max, int num_bins)
{
    if (num_bins <= 2)
        return;
    QVector<double> bin_centers(num_bins);
    for (int i = 0; i < num_bins; i++) {
        bin_centers[i] = d->transform1(bin_min) + (d->transform1(bin_max) - d->transform1(bin_min)) * i * 1.0F / (num_bins - 1);
    }
    qDebug() << __FUNCTION__ << __FILE__ << __LINE__<< "setBins";
    qDebug() << bin_centers;
    this->setBins(bin_centers);
}

void HistogramView::autoSetBins(int num_bins)
{
    if (d->m_data.isEmpty())
        return;
    double data_min = d->m_data[0], data_max = d->m_data[1];
    for (int i = 0; i < d->m_data.count(); i++) {
        if (d->m_data[i] < data_min)
            data_min = d->m_data[i];
        if (d->m_data[i] > data_max)
            data_max = d->m_data[i];
    }
    if (data_max == data_min) {
        data_min -= 1;
        data_max += 1;
    }
    double bin_min = data_min;
    double bin_max = data_max;
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

void HistogramView::setCaption(const QString& caption)
{
    d->m_caption = caption;
    update();
}

void HistogramView::setColors(const QMap<QString, QColor>& colors)
{
    d->m_colors = colors;
}

void HistogramView::setLogMode(bool val, double log_timescale)
{
    if ((d->m_log_mode==val)&&(d->m_log_timescale==log_timescale)) return;
    d->m_log_mode=val;
    d->m_log_timescale=log_timescale;
    d->m_update_required=true;
    this->update();
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

#include "mlcommon.h"
void HistogramView::autoCenterXRange()
{
    double mean_value = MLCompute::mean(d->m_data);
    MVRange xrange = this->xRange();
    double center1 = (xrange.min + xrange.max) / 2;
    xrange = xrange + (mean_value - center1);
    this->setXRange(xrange);
}

void HistogramView::setDrawVerticalAxisAtZero(bool val)
{
    if (d->m_draw_vertical_axis_at_zero == val)
        return;
    d->m_draw_vertical_axis_at_zero = val;
    update();
}

void HistogramView::addVerticalLine(double val)
{
    d->m_vertical_lines << val;
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
    double x = qMin(p1.x(), p2.x());
    double y = qMin(p1.y(), p2.y());
    double w = qAbs(p2.x() - p1.x());
    double h = qAbs(p2.y() - p1.y());
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
    if (evt->button() == Qt::LeftButton)
        emit clicked(evt->modifiers());
    else if (evt->button() == Qt::RightButton)
        emit rightClicked(evt->modifiers());
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
    emit activated(evt->pos());
}

void HistogramView::slot_context_menu(const QPoint& pos)
{
    QMenu M;
    QAction* export_image = M.addAction("Export Histogram Image");
    QAction* export_matrix_image = M.addAction("Export Histogram Matrix Image");
    QAction* selected = M.exec(this->mapToGlobal(pos));
    if (selected == export_image) {
        d->export_image();
    }
    else if (selected == export_matrix_image) {
        emit this->signalExportHistogramMatrixImage();
    }
}

double HistogramViewPrivate::transform1(double t)
{
    //return t;
    double tau=m_log_timescale;

    if (t<0) return -transform1(-t);
    return sqrt(t);
    if (t<=tau) return t;
    else return (1+log(t/tau))*tau;
}

double HistogramViewPrivate::transform2(double x)
{
    //return x;
    double tau=m_log_timescale;
    if (x<0) return -transform2(-x);
    return x*x;
    if (x<=tau) return x;
    else return tau*exp(x/tau-1);
}

QVector<double> HistogramViewPrivate::transform1(const QVector<double> &t)
{
    QVector<double> ret=t;
    for (long i=0; i<t.count(); i++) {
        ret[i]=transform1(ret[i]);
    }
    return ret;
}

void HistogramViewPrivate::update_bin_counts()
{
    for (int i = 0; i < m_bin_centers.count(); i++) {
        m_bin_counts[i] = 0;
        m_second_bin_counts[i] = 0;
        m_bin_densities[i]=0;
        m_second_bin_densities[i]=0;
    }
    for (int pass = 1; pass <= 2; pass++) {
        QVector<double> list;
        if (pass == 1) {
            //list=transform1(m_data);
            list=m_data;
        }
        else {
            //list = transform1(m_second_data);
            list = m_second_data;
        }
        qSort(list);
        if (m_bin_centers.count() < 2)
            return;
        int jj = 0;
        int num_bins=m_bin_centers.count();
        for (int i = 0; i < list.count(); i++) {
            double val = list[i];
            while ((jj + 1 < num_bins) && (m_bin_rights[jj] < val)) {
                jj++;
            }
            if ((val >= transform2(m_bin_lefts[jj])) && (val <= transform2(m_bin_rights[jj]))) {
                if (pass == 1) {
                    m_bin_counts[jj]++;
                }
                else {
                    m_second_bin_counts[jj]++;
                }
            }
        }
    }
    for (int i=0; i<m_bin_counts.count(); i++) {
        double len=transform2(m_bin_rights[i])-transform2(m_bin_lefts[i]);
        qDebug() << "len" << len << "count" << m_bin_counts[i] << m_bin_lefts[i] << m_bin_rights[i] << transform2(m_bin_lefts[i]) << transform2(m_bin_rights[i]);
        if (!len) len=1;
        m_bin_densities[i]=m_bin_counts[i]/len;
        m_second_bin_densities[i]=m_second_bin_counts[i]/len;
    }
    m_max_bin_density=qMax(MLCompute::max(m_bin_densities),MLCompute::max(m_second_bin_densities));
}

QPointF HistogramViewPrivate::coord2pix(QPointF pt, int W, int H)
{
    if (!W)
        W = q->width();
    if (!H)
        H = q->height();

    if (m_bin_centers.count() <= 2) {
        return QPointF(0, 0);
    }
    if ((!W) || (!H))
        return QPointF(0, 0);
    if (W <= m_margin_left + m_margin_right + 5)
        return QPointF(0, 0);
    if (H <= m_margin_top + m_margin_bottom + 5)
        return QPointF(0, 0);

    double xmin = m_bin_lefts[0];
    double xmax = m_bin_rights[m_bin_rights.count()-1];
    double ymin = 0;
    double ymax = m_max_bin_density;

    if (m_xrange.min != m_xrange.max) {
        xmin = transform1(m_xrange.min);
        xmax = transform1(m_xrange.max);
    }

    double xfrac = 0.5;

    //if (!m_log_mode) {
        if (xmax > xmin) {
            xfrac = (transform2(pt.x()) - transform2(xmin)) / (transform2(xmax) - transform2(xmin));
            xfrac = (pt.x() - xmin) / (xmax - xmin);
        }
    //}
    /*
    else {
        double m_logscale_min=1;
        double logscale0=m_logscale_min;
        if (xmax>xmin) {
            double pp=(0-xmin)/(xmax-xmin);
            if ((pt.x()>0)&&(xmax>0)) {
                xfrac=qMax(0.0,(log(pt.x())-log(logscale0))/(log(xmax)-log(logscale0)));
                xfrac=pp+(1-pp)*xfrac;
            }
            else if ((pt.x()<0)&&(xmin<0)) {
                xfrac=qMax(0.0,(log(-pt.x())-log(logscale0))/(log(-xmin)-log(logscale0)));
                xfrac=pp-(pp)*xfrac;
            }
            else xfrac=pp;
        }
        else xfrac=0.5;
    }
    */

    double yfrac = 0.5;
    if (ymax > ymin)
        yfrac = (pt.y() - ymin) / (ymax - ymin);

    double x0 = m_margin_left + xfrac * (W - m_margin_left - m_margin_right);
    double y0 = H - (m_margin_bottom + yfrac * (H - m_margin_top - m_margin_bottom));

    return QPointF(x0, y0);
}

QPointF HistogramViewPrivate::pix2coord(QPointF pt, int W, int H)
{

    if (!W)
        W = q->width();
    if (!H)
        H = q->height();

    if (m_bin_centers.count() <= 2) {
        return QPointF(0, 0);
    }
    if ((!W) || (!H))
        return QPointF(0, 0);
    if (W <= m_margin_left + m_margin_right + 5)
        return QPointF(0, 0);
    if (H <= m_margin_top + m_margin_bottom + 5)
        return QPointF(0, 0);

    double xmin = m_bin_lefts[0];
    double xmax = m_bin_rights[m_bin_rights.count() - 1];
    double ymin = 0;
    double ymax = m_max_bin_density;

    if (m_xrange.min != m_xrange.max) {
        xmin = m_xrange.min;
        xmax = m_xrange.max;
    }

    double xfrac = (pt.x() - m_margin_left) / (W - m_margin_left - m_margin_right);
    double yfrac = 1 - (pt.y() - m_margin_top) / (H - m_margin_top - m_margin_bottom);

    double x0 = xmin + xfrac * (xmax - xmin);
    double y0 = ymin + yfrac * (ymax - ymin);

    return QPointF(x0, y0);
}

int HistogramViewPrivate::get_bin_index_at(QPointF pt_pix)
{
    if (m_bin_centers.count() <= 2) {
        return -1;
    }
    QPointF pt = pix2coord(pt_pix);
    for (int i = 0; i < m_bin_centers.count(); i++) {
        if ((pt.x() >= m_bin_lefts[i]) && (pt.x() <= m_bin_rights[i])) {
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

QColor modify_color_for_second_histogram(QColor col)
{
    QColor ret = col;
    ret.setGreen(qMin(255, ret.green() + 30)); //more green
    ret = lighten(ret, -20, -20, -20); //darker
    return ret;
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
    }
    else if (m_selected) {
        painter.fillRect(R, m_colors["view_background_selected"]);
    }
    else if (m_hovered) {
        painter.fillRect(R, m_colors["view_background_hovered"]);
    }
    else {
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

    if (m_bin_centers.count() <= 2)
        return;

    for (int pass = 1; pass <= 2; pass++) {
        QVector<double> *bin_densities;
        if (pass == 1)
            bin_densities = &m_bin_densities;
        else
            bin_densities = &m_second_bin_densities;
        QColor col = m_fill_color;
        QColor line_color = m_line_color;
        if (pass == 2) {
            col = modify_color_for_second_histogram(col);
            line_color = modify_color_for_second_histogram(line_color);
        }
        for (int i = 0; i < m_bin_centers.count(); i++) {
            QPointF pt1 = coord2pix(QPointF(m_bin_lefts[i], 0), W, H);
            QPointF pt2 = coord2pix(QPointF(m_bin_rights[i], (*bin_densities)[i]), W, H);
            QRectF R = make_rect(pt1, pt2);
            if (i == m_hovered_bin_index)
                painter.fillRect(R, lighten(col, 25, 25, 25));
            else
                painter.fillRect(R, col);
            painter.setPen(line_color);
            painter.drawRect(R);
        }
    }

    if (m_draw_vertical_axis_at_zero) {
        QPointF pt0 = coord2pix(QPointF(0, 0));
        QPointF pt1 = coord2pix(QPointF(0, m_max_bin_density));
        QPen pen = painter.pen();
        pen.setColor(Qt::black);
        painter.setPen(pen);
        painter.drawLine(pt0, pt1);
    }

    foreach (double val, m_vertical_lines) {
        QPointF pt0 = coord2pix(QPointF(val, 0));
        QPointF pt1 = coord2pix(QPointF(val, m_max_bin_density));
        QPen pen = painter.pen();
        pen.setColor(Qt::gray);
        pen.setStyle(Qt::DashLine);
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
    if (!m_caption.isEmpty()) {
        int text_height = 12;
        QRect R(0, H - m_margin_bottom, W, m_margin_bottom);
        QFont font = painter.font();
        font.setFamily("Arial");
        font.setPixelSize(text_height);
        painter.setFont(font);
        painter.setPen(QColor(100, 60, 60));
        painter.drawText(R, m_caption, Qt::AlignCenter | Qt::AlignVCenter);
    }
}
