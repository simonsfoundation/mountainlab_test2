/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 8/24/2016
*******************************************************/

#include "curationprogramplugin.h"

#include "curationprogramview.h"

#include <QThread>
#include <mountainprocessrunner.h>

class CurationProgramPluginPrivate {
public:
    CurationProgramPlugin* q;
};

CurationProgramPlugin::CurationProgramPlugin()
{
    d = new CurationProgramPluginPrivate;
    d->q = this;
}

CurationProgramPlugin::~CurationProgramPlugin()
{
    delete d;
}

QString CurationProgramPlugin::name()
{
    return "CurationProgram";
}

QString CurationProgramPlugin::description()
{
    return "";
}

void compute_basic_metrics(MVContext* mv_context);
void CurationProgramPlugin::initialize(MVMainWindow* mw)
{
    mw->registerViewFactory(new CurationProgramFactory(mw));
    compute_basic_metrics(mw->mvContext());
}

CurationProgramFactory::CurationProgramFactory(MVMainWindow* mw, QObject* parent)
    : MVAbstractViewFactory(mw, parent)
{
}

QString CurationProgramFactory::id() const
{
    return QStringLiteral("open-curation-program");
}

QString CurationProgramFactory::name() const
{
    return tr("Curation Program");
}

QString CurationProgramFactory::title() const
{
    return tr("Curation Program");
}

MVAbstractView* CurationProgramFactory::createView(MVContext* context)
{
    CurationProgramView* X = new CurationProgramView(context);
    return X;
}
