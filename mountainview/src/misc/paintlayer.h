/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 7/1/2016
*******************************************************/

#ifndef PAINTLAYER_H
#define PAINTLAYER_H

#include <QMouseEvent>
#include <QPainter>

class PaintLayerPrivate;
class PaintLayer : public QObject {
    Q_OBJECT
public:
    friend class PaintLayerPrivate;
    PaintLayer(QObject* parent = 0);
    virtual ~PaintLayer();

    virtual void paint(QPainter* painter) = 0;
    virtual void mousePressEvent(QMouseEvent* evt) { Q_UNUSED(evt) }
    virtual void mouseReleaseEvent(QMouseEvent* evt) { Q_UNUSED(evt) }
    virtual void mouseMoveEvent(QMouseEvent* evt) { Q_UNUSED(evt) }

    //used by the subclass
    QSize windowSize() const;

    //used by the widget/paintlayerstack
    virtual void setWindowSize(QSize size);

signals:
    void windowSizeChanged();
    void repaintNeeded();

private:
    PaintLayerPrivate* d;
};

#endif // PAINTLAYER_H
