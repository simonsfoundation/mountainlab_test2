/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 6/27/2016
*******************************************************/

#include "mvclustervisibilitycontrol.h"
#include "mvviewagent.h"

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMap>
#include <QTimer>
#include "flowlayout.h"

class MVClusterVisibilityControlPrivate {
public:
    MVClusterVisibilityControl* q;
    FlowLayout* m_flow_layout;
    QWidgetList m_all_widgets;
};

MVClusterVisibilityControl::MVClusterVisibilityControl(MVContext* context, MVMainWindow* mw)
    : MVAbstractControl(context, mw)
{
    d = new MVClusterVisibilityControlPrivate;
    d->q = this;

    d->m_flow_layout = new FlowLayout;

    this->setLayout(d->m_flow_layout);

    QObject::connect(context, SIGNAL(clusterVisibilityChanged()), this, SLOT(updateControls()));

    updateControls();
}

MVClusterVisibilityControl::~MVClusterVisibilityControl()
{
    delete d;
}

QString MVClusterVisibilityControl::title()
{
    return "Cluster Visibility";
}

void MVClusterVisibilityControl::updateContext()
{
    ClusterVisibilityRule rule = mvContext()->visibilityRule();

    rule.view_all_tagged = this->controlValue("all_tagged").toBool();
    rule.view_all_untagged = this->controlValue("all_untagged").toBool();

    rule.view_tags.clear();
    QStringList tags = mvContext()->allClusterTags().toList();
    foreach (QString tag, tags) {
        if (this->controlValue("tag-" + tag).toBool()) {
            rule.view_tags << tag;
        }
    }

    mvContext()->setVisibilityRule(rule);
}

void MVClusterVisibilityControl::updateControls()
{
    ClusterVisibilityRule rule = mvContext()->visibilityRule();

    QStringList tags = mvContext()->allClusterTags().toList();
    qSort(tags);

    qDeleteAll(d->m_all_widgets);
    d->m_all_widgets.clear();

    {
        QCheckBox* CB = this->createCheckBoxControl("all_tagged");
        CB->setText("All tagged");
        CB->setChecked(rule.view_all_tagged);
        d->m_flow_layout->addWidget(CB);
        d->m_all_widgets << CB;
    }
    {
        QCheckBox* CB = this->createCheckBoxControl("all_untagged");
        CB->setText("All untagged");
        CB->setChecked(rule.view_all_untagged);
        d->m_flow_layout->addWidget(CB);
        d->m_all_widgets << CB;
    }
    foreach (QString tag, tags) {
        QCheckBox* CB = this->createCheckBoxControl("tag-" + tag);
        CB->setText(tag);
        CB->setChecked(rule.view_tags.contains(tag));
        d->m_flow_layout->addWidget(CB);
        d->m_all_widgets << CB;
        if (rule.view_all_tagged)
            CB->setEnabled(false);
    }
}
