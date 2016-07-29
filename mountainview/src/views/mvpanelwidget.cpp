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
    int row=0;
    int col=0;
    PaintLayer *layer=0;
    QRectF geom=QRectF(0,0,0,0);
};

class MVPanelWidgetPrivate {
public:
    MVPanelWidget *q;

    QList<MVPanelWidgetPanel> m_panels;
    int m_row_margin=3;
    int m_row_spacing=3;
    int m_col_margin=3;
    int m_col_spacing=3;
    QRectF m_viewport_geom=QRectF(0,0,1,1);
    int m_current_panel_index=-1;

    void correct_viewport_geom();
};

MVPanelWidget::MVPanelWidget()
{
    d=new MVPanelWidgetPrivate;
    d->q=this;
}

MVPanelWidget::~MVPanelWidget()
{
    delete d;
}

void MVPanelWidget::clearPanels(bool delete_layers)
{
    if (delete_layers) {
        foreach (MVPanelWidgetPanel panel,d->m_panels) {
            delete panel.layer;
        }
    }
    d->m_panels.clear();
    update();
}

void MVPanelWidget::addPanel(int row, int col, PaintLayer *layer)
{
    MVPanelWidgetPanel panel;
    panel.row=row;
    panel.col=col;
    panel.layer=layer;
    d->m_panels << panel;
    update();
}

int MVPanelWidget::rowCount() const
{
    int ret=0;
    for (int i=0; i<d->m_panels.count(); i++) {
        ret=qMax(ret,d->m_panels[i].row+1);
    }
    return ret;
}

int MVPanelWidget::columnCount() const
{
    int ret=0;
    for (int i=0; i<d->m_panels.count(); i++) {
        ret=qMax(ret,d->m_panels[i].col+1);
    }
    return ret;
}

void MVPanelWidget::setSpacing(int row_spacing, int col_spacing)
{
    d->m_row_spacing=row_spacing;
    d->m_col_spacing=col_spacing;
    update();
}

void MVPanelWidget::setMargins(int row_margin, int col_margin)
{
    d->m_row_margin=row_margin;
    d->m_col_margin=col_margin;
    update();
}

void MVPanelWidget::setViewportGeometry(QRectF geom)
{
    d->m_viewport_geom=geom;
    update();
}

void MVPanelWidget::paintEvent(QPaintEvent *evt)
{
    Q_UNUSED(evt)
    QPainter painter(this);
    d->correct_viewport_geom();
    int num_rows=this->rowCount();
    int num_cols=this->columnCount();
    if (!num_rows) return;
    if (!num_cols) return;
    double viewport_W=this->width();
    double viewport_H=this->height();
    QRectF VG=d->m_viewport_geom;
    QRectF vgeom=QRectF(VG.left()*viewport_W,VG.top()*viewport_H,VG.width()*viewport_W,VG.height()*viewport_H);
    double W0=(vgeom.width()-(num_cols-1)*d->m_col_spacing-d->m_col_margin*2)/num_cols;
    double H0=(vgeom.height()-(num_rows-1)*d->m_row_spacing-d->m_row_margin*2)/num_rows;
    for (int i=0; i<d->m_panels.count(); i++) {
        double x1=vgeom.x()+d->m_col_margin+d->m_panels[i].col*(W0+d->m_col_spacing);
        double y1=vgeom.y()+d->m_row_margin+d->m_panels[i].row*(H0+d->m_row_spacing);
        d->m_panels[i].geom=QRectF(x1,y1,W0,H0);
        d->m_panels[i].layer->setWindowSize(QSize(W0,H0));
        painter.translate(QPointF(x1,y1));
        d->m_panels[i].layer->paint(&painter);
        painter.translate(QPointF(-x1,-y1));
    }
}


void MVPanelWidgetPrivate::correct_viewport_geom()
{
    QRectF G=m_viewport_geom;
    if (G.width()<1) {
        G.setLeft(0);
        G.setWidth(1);
    }
    if (G.left()+G.width()<1) G.setLeft(1-G.width());
    if (G.left()>0) G.setLeft(0);
    if (G.height()<1) {
        G.setTop(0);
        G.setHeight(1);
    }
    if (G.top()+G.height()<1) G.setTop(1-G.height());
    if (G.top()>0) G.setTop(0);
    m_viewport_geom=G;
}
