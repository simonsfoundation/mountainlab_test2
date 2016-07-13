#include "sslabelview.h"

#include "sslabelsmodel1_prev.h"
#include "sslabelplot_prev.h"
#include <QVBoxLayout>
#include <QDebug>

class SSLabelViewPrivate {
public:
    SSLabelView* q;
    SSLabelPlot* m_plot;
    SSLabelsModel* m_labels;
    bool m_labels_is_owner;
};

SSLabelView::SSLabelView()
{
    d = new SSLabelViewPrivate;
    d->q = this;

    d->m_labels = 0;
    d->m_labels_is_owner = false;

    d->m_plot = new SSLabelPlot;
    d->m_plot->setMargins(0, 0, 0, 30);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
    layout->addWidget(plot());
}

SSLabelView::~SSLabelView()
{
    delete d;
}

void SSLabelView::setLabels(DiskReadMdaOld* TL, bool is_owner)
{
    SSLabelsModel1* L = new SSLabelsModel1;
    L->setTimepointsLabels(TL, is_owner);
    setLabels(L, true);
}

void SSLabelView::setLabels(SSLabelsModel* L, bool is_owner)
{
    d->m_labels = L;
    d->m_labels_is_owner = is_owner;
    d->m_plot->setLabels(L);

    setMaxTimepoint(L->getMaxTimepoint());

    this->setCurrentX(-1);
}

SSAbstractPlot* SSLabelView::plot()
{
    return d->m_plot;
}

QString SSLabelView::viewType()
{
    return "SSLabelView";
}
