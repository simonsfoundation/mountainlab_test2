/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 7/29/2016
*******************************************************/

#include "mvpanelwidget.h"
#include "paintlayerstack.h"
#include <QRectF>
#include <QDebug>

struct MVPanelWidgetPanel {
    int row = 0;
    int col = 0;
    PaintLayer* layer = 0;
    QRectF geom = QRectF(0, 0, 0, 0);
};

class MVPanelWidgetPrivate {
public:
    MVPanelWidget* q;

    QList<MVPanelWidgetPanel> m_panels;
    int m_row_margin = 3;
    int m_row_spacing = 3;
    int m_col_margin = 3;
    int m_col_spacing = 3;
    double m_max_panel_width = 600;
    QRectF m_viewport_geom = QRectF(0, 0, 1, 1);
    int m_current_panel_index = -1;

    void correct_viewport_geom();
    void zoom(double factor);
    void update_panel_geometries();
    int panel_index_at_pt(const QPointF& pt);
};

MVPanelWidget::MVPanelWidget()
{
    d = new MVPanelWidgetPrivate;
    d->q = this;
}

MVPanelWidget::~MVPanelWidget()
{
    delete d;
}

void MVPanelWidget::clearPanels(bool delete_layers)
{
    if (delete_layers) {
        foreach (MVPanelWidgetPanel panel, d->m_panels) {
            delete panel.layer;
        }
    }
    d->m_panels.clear();
    update();
}

void MVPanelWidget::addPanel(int row, int col, PaintLayer* layer)
{
    MVPanelWidgetPanel panel;
    panel.row = row;
    panel.col = col;
    panel.layer = layer;
    d->m_panels << panel;
    update();
}

int MVPanelWidget::rowCount() const
{
    int ret = 0;
    for (int i = 0; i < d->m_panels.count(); i++) {
        ret = qMax(ret, d->m_panels[i].row + 1);
    }
    return ret;
}

int MVPanelWidget::columnCount() const
{
    int ret = 0;
    for (int i = 0; i < d->m_panels.count(); i++) {
        ret = qMax(ret, d->m_panels[i].col + 1);
    }
    return ret;
}

void MVPanelWidget::setSpacing(int row_spacing, int col_spacing)
{
    d->m_row_spacing = row_spacing;
    d->m_col_spacing = col_spacing;
    update();
}

void MVPanelWidget::setMargins(int row_margin, int col_margin)
{
    d->m_row_margin = row_margin;
    d->m_col_margin = col_margin;
    update();
}

void MVPanelWidget::setViewportGeometry(QRectF geom)
{
    d->m_viewport_geom = geom;
    update();
}

void MVPanelWidget::paintEvent(QPaintEvent* evt)
{
    Q_UNUSED(evt)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    d->correct_viewport_geom();
    d->update_panel_geometries();

    for (int i = 0; i < d->m_panels.count(); i++) {
        QRectF geom = d->m_panels[i].geom;
        d->m_panels[i].layer->setWindowSize(geom.size().toSize());
        double x1 = geom.left();
        double y1 = geom.top();
        painter.translate(QPointF(x1, y1));
        d->m_panels[i].layer->paint(&painter);
        painter.translate(QPointF(-x1, -y1));
    }
}

void MVPanelWidget::wheelEvent(QWheelEvent* evt)
{
    int delta = evt->delta();
    double factor = 1;
    if (delta > 0)
        factor = 1.2;
    else
        factor = 1 / 1.2;
    d->zoom(factor);
}

void MVPanelWidget::mousePressEvent(QMouseEvent* evt)
{
    qDebug() << __FUNCTION__ << __FILE__ << __LINE__;
    int index = d->panel_index_at_pt(evt->pos());
    qDebug() << __FUNCTION__ << __FILE__ << __LINE__ << ":::::::::::::::" << index;
    if (index >= 0) {
        d->m_current_panel_index = index;
        update();
    }
    QWidget::mousePressEvent(evt);
}

void MVPanelWidgetPrivate::correct_viewport_geom()
{
    QRectF G = m_viewport_geom;
    if (q->width() * G.width() / m_panels.count() > m_max_panel_width) {
        G.setWidth(m_max_panel_width * m_panels.count() / q->width());
    }
    if (G.width() < 1) {
        G.setLeft(0);
        G.setWidth(1);
    }
    if (G.left() + G.width() < 1)
        G.setLeft(1 - G.width());
    if (G.left() > 0)
        G.setLeft(0);
    if (G.height() < 1) {
        G.setTop(0);
        G.setHeight(1);
    }
    if (G.top() + G.height() < 1)
        G.setTop(1 - G.height());
    if (G.top() > 0)
        G.setTop(0);
    m_viewport_geom = G;
}

void MVPanelWidgetPrivate::zoom(double factor)
{
    QRectF current_panel_geom = QRectF(0, 0, 1, 1);
    if ((m_current_panel_index >= 0) && (m_current_panel_index < m_panels.count())) {
        current_panel_geom = m_panels[m_current_panel_index].geom;
    }
    if (q->rowCount() > 1) {
        m_viewport_geom.setHeight(m_viewport_geom.height() * factor);
    }
    if (q->columnCount() > 1) {
        m_viewport_geom.setWidth(m_viewport_geom.width() * factor);
    }
    correct_viewport_geom();
    update_panel_geometries();
    QRectF new_current_panel_geom = QRectF(0, 0, 1, 1);
    if ((m_current_panel_index >= 0) && (m_current_panel_index < m_panels.count())) {
        new_current_panel_geom = m_panels[m_current_panel_index].geom;
    }
    double dx = new_current_panel_geom.center().x() - current_panel_geom.center().x();
    double dy = new_current_panel_geom.center().y() - current_panel_geom.center().y();
    if ((dx) || (dy)) {
        m_viewport_geom.setLeft(m_viewport_geom.left() - dx / q->width());
        m_viewport_geom.setTop(m_viewport_geom.top() - dy / q->height());
    }
    correct_viewport_geom();
    update_panel_geometries();
    q->update();
}

void MVPanelWidgetPrivate::update_panel_geometries()
{
    int num_rows = q->rowCount();
    int num_cols = q->columnCount();
    if (!num_rows)
        return;
    if (!num_cols)
        return;
    double viewport_W = q->width();
    double viewport_H = q->height();
    QRectF VG = m_viewport_geom;
    QRectF vgeom = QRectF(VG.left() * viewport_W, VG.top() * viewport_H, VG.width() * viewport_W, VG.height() * viewport_H);
    double W0 = (vgeom.width() - (num_cols - 1) * m_col_spacing - m_col_margin * 2) / num_cols;
    double H0 = (vgeom.height() - (num_rows - 1) * m_row_spacing - m_row_margin * 2) / num_rows;
    for (int i = 0; i < m_panels.count(); i++) {
        double x1 = vgeom.x() + m_col_margin + m_panels[i].col * (W0 + m_col_spacing);
        double y1 = vgeom.y() + m_row_margin + m_panels[i].row * (H0 + m_row_spacing);
        m_panels[i].geom = QRectF(x1, y1, W0, H0);
    }
}

int MVPanelWidgetPrivate::panel_index_at_pt(const QPointF& pt)
{
    for (int i = 0; i < m_panels.count(); i++) {
        if (m_panels[i].geom.contains(pt))
            return i;
    }
    return -1;
}
