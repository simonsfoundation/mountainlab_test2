/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 7/12/2016
*******************************************************/

#include "firetrackview.h"
#include "ftelectrodearrayview.h"

#include <QHBoxLayout>

/// TODO show current cluster waveform in firetrackview

class FireTrackViewPrivate {
public:
    FireTrackView* q;

    FTElectrodeArrayView* m_electrode_array_view;
};

FireTrackView::FireTrackView(MVContext* context)
    : MVAbstractView(context)
{
    d = new FireTrackViewPrivate;
    d->q = this;

    d->m_electrode_array_view = new FTElectrodeArrayView;
    d->m_electrode_array_view->setShowChannelNumbers(true);

    QHBoxLayout* layout = new QHBoxLayout;
    layout->addWidget(d->m_electrode_array_view);

    this->setLayout(layout);

    this->recalculateOn(context, SIGNAL(electrodeGeometryChanged()));

    recalculate();
}

FireTrackView::~FireTrackView()
{
    delete d;
}

void FireTrackView::prepareCalculation()
{
}

void FireTrackView::runCalculation()
{
}

void FireTrackView::onCalculationFinished()
{
    ElectrodeGeometry geom = mvContext()->electrodeGeometry();
    Mda locations;
    for (int i = 0; i < geom.coordinates.count(); i++) {
        if (i == 0) {
            locations.allocate(geom.coordinates.count(), geom.coordinates[i].count());
        }
        for (int j = 0; j < locations.N1(); j++) {
            locations.setValue(geom.coordinates[i][j], i, j);
        }
    }
    d->m_electrode_array_view->setElectrodeLocations(locations);
}

MVFireTrackFactory::MVFireTrackFactory(QObject* parent)
    : MVAbstractViewFactory(parent)
{
}

QString MVFireTrackFactory::id() const
{
    return QStringLiteral("open-firetrack");
}

QString MVFireTrackFactory::name() const
{
    return tr("FireTrack");
}

QString MVFireTrackFactory::title() const
{
    return tr("FireTrack");
}

MVAbstractView* MVFireTrackFactory::createView(MVContext* context)
{
    FireTrackView* X = new FireTrackView(context);
    return X;
}
