/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 7/1/2016
*******************************************************/

#include "paintlayer.h"

class PaintLayerPrivate {
public:
    PaintLayer* q;
    bool m_update_needed = false;
    QSize m_window_size = QSize(0, 0);
};

PaintLayer::PaintLayer(QObject* parent)
    : QObject(parent)
{
    d = new PaintLayerPrivate;
    d->q = this;
}

PaintLayer::~PaintLayer()
{
    delete d;
}

QSize PaintLayer::windowSize() const
{
    return d->m_window_size;
}

void PaintLayer::setUpdateNeeded(bool val)
{
    if (d->m_update_needed == val)
        return;
    d->m_update_needed = val;
    if (val)
        emit signalUpdateNeeded();
}

void PaintLayer::setWindowSize(QSize size)
{
    if (d->m_window_size == size)
        return;
    d->m_window_size = size;
    emit this->signalWindowSizeChanged();
}

bool PaintLayer::updateNeeded() const
{
    return d->m_update_needed;
}
