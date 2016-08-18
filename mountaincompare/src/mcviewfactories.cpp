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

    MVClusterDetailWidget* X = new MVClusterDetailWidget(c2);
    return X;
}
