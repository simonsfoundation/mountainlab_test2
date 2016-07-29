/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 7/29/2016
*******************************************************/

#include "mvtemplatesview2panel.h"
#include "mlcommon.h"

class MVTemplatesView2PanelPrivate {
public:
    MVTemplatesView2Panel* q;
    Mda m_template;
    ElectrodeGeometry m_electrode_geometry;
    QList<QColor> m_channel_colors;
    QList<QRectF> m_electrode_boxes;
    int m_clip_size = 0;
    double m_vertical_scale_factor = 1;

    void setup_electrode_boxes(double W, double H);
    QPointF coord2pix(int m, int t, double val);
};

MVTemplatesView2Panel::MVTemplatesView2Panel()
{
    d = new MVTemplatesView2PanelPrivate;
    d->q = this;
}

MVTemplatesView2Panel::~MVTemplatesView2Panel()
{
    delete d;
}

void MVTemplatesView2Panel::setTemplate(const Mda& X)
{
    d->m_template = X;
}

void MVTemplatesView2Panel::setElectrodeGeometry(const ElectrodeGeometry& geom)
{
    d->m_electrode_geometry = geom;
}

void MVTemplatesView2Panel::setVerticalScaleFactor(double factor)
{
    d->m_vertical_scale_factor = factor;
}

void MVTemplatesView2Panel::setChannelColors(const QList<QColor>& colors)
{
    d->m_channel_colors = colors;
}

void MVTemplatesView2Panel::paint(QPainter* painter)
{
    QSize ss = this->windowSize();
    QPen pen = painter->pen();

    d->setup_electrode_boxes(ss.width(), ss.height());

    //draw box
    {
        pen.setColor(Qt::gray);
        pen.setWidth(1);
        painter->setPen(pen);
        painter->drawRect(0, 0, ss.width(), ss.height());
    }

    int M = d->m_template.N1();
    int T = d->m_template.N2();
    d->m_clip_size = T;
    for (int m = 0; m < M; m++) {
        QPainterPath path;
        for (int t = 0; t < T; t++) {
            double val = d->m_template.value(m, t);
            QPointF pt = d->coord2pix(m, t, val);
            if (t == 0)
                path.moveTo(pt);
            else
                path.lineTo(pt);
        }
        pen.setColor(d->m_channel_colors.value(m, Qt::black));
        pen.setWidth(2);
        painter->strokePath(path, pen);
        QRectF box = d->m_electrode_boxes.value(m);
        painter->drawEllipse(box);
    }
}

double estimate_spacing(const QList<QVector<double> >& coords)
{
    QVector<double> vals;
    for (int m = 0; m < coords.length(); m++) {
        QVector<double> dists;
        for (int i = 0; i < coords.length(); i++) {
            if (i != m) {
                double dx = coords[i].value(0) - coords[m].value(0);
                double dy = coords[i].value(1) - coords[m].value(1);
                double dist = sqrt(dx * dx + dy * dy);
                dists << dist;
            }
        }
        vals << MLCompute::min(dists);
    }
    return MLCompute::mean(vals);
}

void MVTemplatesView2PanelPrivate::setup_electrode_boxes(double W, double H)
{
    m_electrode_boxes.clear();

    QList<QVector<double> > coords = m_electrode_geometry.coordinates;
    if (coords.isEmpty())
        return;

    int D = coords[0].count();
    QVector<double> mins(D), maxs(D);
    for (int d = 0; d < D; d++) {
        mins[d] = maxs[d] = 0;
    }
    for (int m = 0; m < coords.count(); m++) {
        for (int d = 0; d < D; d++) {
            mins[d] = qMin(mins[d], coords[m][d]);
            maxs[d] = qMax(maxs[d], coords[m][d]);
        }
    }

    double spacing = estimate_spacing(coords); //compute this more better
    for (int d = 0; d < D; d++) {
        mins[d] -= spacing;
        maxs[d] += spacing;
    }

    double W0 = maxs.value(0) - mins.value(0);
    double H0 = maxs.value(1) - mins.value(1);

    double scale_factor = 1;
    if (W0 * H > W * H0) {
        //limited by width
        if (W0)
            scale_factor = W / W0;
    }
    else {
        if (H0)
            scale_factor = H / H0;
    }

    for (int m = 0; m < coords.count(); m++) {
        QVector<double> c = coords[m];
        double x0 = (c.value(0) - mins.value(0)) * scale_factor;
        double y0 = (c.value(1) - mins.value(1)) * scale_factor;
        double radx = spacing * scale_factor / 2;
        double rady = spacing * scale_factor / 3.5;
        m_electrode_boxes << QRectF(x0 - radx, y0 - rady, radx * 2, rady * 2);
    }
}

QPointF MVTemplatesView2PanelPrivate::coord2pix(int m, int t, double val)
{
    QRectF R = m_electrode_boxes.value(m);
    QPointF Rcenter = R.center();
    double pctx = t * 1.0 / m_clip_size;
    double x0 = R.left() + (R.width()) * pctx;
    double y0 = Rcenter.y() - val * R.height() / 2 * m_vertical_scale_factor;
    return QPointF(x0, y0);
}
