#include "matrixview.h"

#include <mvmisc.h>

class MatrixViewPrivate {
public:
    MatrixView *q;
    Mda m_matrix;
    MVRange m_value_range=MVRange(0,1);
    MatrixView::Mode m_mode;

    QRectF get_entry_rect(int m,int n);
    QPointF coord2pix(double m,double n);
    QColor get_color(double val);
    void draw_string_in_rect(QPainter &painter,QRectF r,QString txt,QColor col);
    QColor complementary_color(QColor col);
};

MatrixView::MatrixView()
{
    d=new MatrixViewPrivate;
    d->q=this;
}

MatrixView::~MatrixView()
{
    delete d;
}

void MatrixView::setMode(MatrixView::Mode mode)
{
    d->m_mode=mode;
}

void MatrixView::setMatrix(const Mda &A)
{
    d->m_matrix=A;
    update();
}

void MatrixView::setValueRange(double minval, double maxval)
{
    d->m_value_range=MVRange(minval,maxval);
    update();
}

void MatrixView::paintEvent(QPaintEvent *evt)
{
    Q_UNUSED(evt)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    int M=d->m_matrix.N1();
    int N=d->m_matrix.N2();
    for (int n=0; n<N; n++) {
        for (int m=0; m<M; m++) {
            QRectF r=d->get_entry_rect(m,n);
            double val=d->m_matrix.value(m,n);
            QColor col=d->get_color(val);
            painter.fillRect(r,col);
            QString str;
            if (d->m_mode==PercentMode) {
                str=QString("%1%").arg((int)(val*100));
                if (val==0) str="";
            }
            else if (d->m_mode==CountsMode) {
                str=QString("%1").arg(val);
                if (val==0) str="";
            }
            d->draw_string_in_rect(painter,r,str,d->complementary_color(col));
        }
    }
}

QRectF MatrixViewPrivate::get_entry_rect(int m, int n)
{
    QPointF pt1=coord2pix(m,n);
    QPointF pt2=coord2pix(m+1,n+1);
    return QRectF(pt1,pt2);
}

QPointF MatrixViewPrivate::coord2pix(double m, double n)
{
    int W0=q->width();
    int H0=q->height();
    if (!(W0*H0)) return QPointF(-1,-1);
    int M=m_matrix.N1();
    int N=m_matrix.N2();
    if (!(M*N)) return QPointF(0,0);

    return QPointF(n/N*W0,m/M*H0);
}

QColor MatrixViewPrivate::get_color(double val)
{
    if (m_value_range.max<=m_value_range.min) return QColor(0,0,0);
    double tmp=(val-m_value_range.min)/m_value_range.range();
    tmp=qMax(0.0,qMin(1.0,tmp));
    int tmp2=(int)(tmp*255);
    return QColor(tmp2,tmp2,tmp2);
}

void MatrixViewPrivate::draw_string_in_rect(QPainter &painter, QRectF r, QString txt, QColor col)
{
    painter.save();

    QPen pen=painter.pen();
    pen.setColor(col);
    painter.setPen(pen);

    QFont font=painter.font();
    int pix=16;
    font.setPixelSize(pix);
    while ((pix>6)&&(QFontMetrics(font).width(txt)>=r.width())) {
        pix--;
        font.setPixelSize(pix);
    }
    painter.setFont(font);

    painter.setClipRect(r);
    painter.drawText(r,Qt::AlignCenter|Qt::AlignVCenter,txt);

    painter.restore();
}

QColor MatrixViewPrivate::complementary_color(QColor col)
{

    int r=col.red();
    int g=col.green();
    int b=col.blue();

    // (255,255,255) -> (0,0,255)
    // (128,128,128) -> (0,0,255)
    // (0,0,0) -> (180,180,255)

    return QColor(qMax(0,200-r*4),qMax(0,200-g),255);
}
