/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 7/29/2016
*******************************************************/

#include "mvtemplatesview2panel.h"

class MVTemplatesView2PanelPrivate {
public:
    MVTemplatesView2Panel *q;
    Mda m_template;
    ElectrodeGeometry m_electrode_geometry;
    QList<QColor> m_channel_colors;
    QList<QRectF> m_electrode_boxes;

    void setup_electrode_boxes();
    QPointF coord2pix(int m, int t, double val);
};

MVTemplatesView2Panel::MVTemplatesView2Panel()
{
    d=new MVTemplatesView2PanelPrivate;
    d->q=this;
}

MVTemplatesView2Panel::~MVTemplatesView2Panel()
{
    delete d;
}

void MVTemplatesView2Panel::setTemplate(const Mda &X)
{
    d->m_template=X;
}

void MVTemplatesView2Panel::setElectrodeGeometry(const ElectrodeGeometry &geom)
{
    d->m_electrode_geometry=geom;
}

void MVTemplatesView2Panel::channelColors(const QList<QColor> &colors)
{
    d->m_channel_colors=colors;
}

void MVTemplatesView2Panel::paint(QPainter *painter)
{
    QSize ss=this->windowSize();
    QPen pen=painter->pen();

    d->setup_electrode_boxes();

    //draw box
    {
        pen.setColor(Qt::gray);
        painter->setPen(pen);
        painter->drawRect(0,0,ss.width(),ss.height());
    }

    int M=d->m_template.N1();
    int T=d->m_template.N2();
    for (int m=0; m<M; m++) {
        QPainterPath path;
        for (int t=0; t<T; t++) {
            double val=d->m_template.value(m,t);
            QPointF pt=d->coord2pix(m,t,val);
            if (t==0) path.moveTo(pt);
            else path.lineTo(pt);
        }
        pen.setColor(d->m_channel_colors.value(m,Qt::black));
        painter->strokePath(path,pen);
    }
}


void MVTemplatesView2PanelPrivate::setup_electrode_boxes()
{

}

QPointF MVTemplatesView2PanelPrivate::coord2pix(int m, int t, double val)
{

}
