#include "mccontext.h"
#include "mcviewfactories.h"

#include <mvclusterdetailwidget.h>

MVClusterDetails2Factory::MVClusterDetails2Factory(MVContext* context, QObject* parent)
    : MVAbstractViewFactory(context, parent)
{
}

QString MVClusterDetails2Factory::id() const
{
    return QStringLiteral("open-cluster-details-2");
}

QString MVClusterDetails2Factory::name() const
{
    return tr("Cluster Details 2");
}

QString MVClusterDetails2Factory::title() const
{
    return tr("Details 2");
}

MVAbstractView* MVClusterDetails2Factory::createView(QWidget* parent)
{
    Q_UNUSED(parent)

    MCContext *mc_context=qobject_cast<MCContext*>(mvContext());
    if (!mc_context) return 0;

    MVContext *c2=new MVContext;
    c2->setFromMVFileObject(mc_context->toMVFileObject());
    c2->setFirings(mc_context->firings2());
    c2->setCurrentCluster(mc_context->currentCluster2());

    new Synchronizer1(mc_context,c2);

    MVClusterDetailWidget* X = new MVClusterDetailWidget(c2);
    return X;
}

Synchronizer1::Synchronizer1(MCContext *C, MVContext *C_new)
{
    m_C=C;
    m_C_new=C_new;
    QObject::connect(C,SIGNAL(selectedClusters2Changed()),this,SLOT(sync_old_to_new()),Qt::QueuedConnection);
    QObject::connect(C,SIGNAL(currentCluster2Changed()),this,SLOT(sync_old_to_new()),Qt::QueuedConnection);

    QObject::connect(C_new,SIGNAL(selectedClustersChanged()),this,SLOT(sync_new_to_old()),Qt::QueuedConnection);
    QObject::connect(C_new,SIGNAL(currentClusterChanged()),this,SLOT(sync_new_to_old()),Qt::QueuedConnection);
}

void Synchronizer1::sync_new_to_old()
{
    m_C->setSelectedClusters2(m_C_new->selectedClusters());
    m_C->setCurrentCluster2(m_C_new->currentCluster());
}

void Synchronizer1::sync_old_to_new()
{
    m_C_new->setSelectedClusters(m_C->selectedClusters2());
    m_C_new->setCurrentCluster(m_C->currentCluster2());
}

