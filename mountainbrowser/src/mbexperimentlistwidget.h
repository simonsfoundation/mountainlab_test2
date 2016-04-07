/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/6/2016
*******************************************************/

#ifndef MBEXPERIMENTLIST_H
#define MBEXPERIMENTLIST_H

#include "mbexperimentmanager.h"

#include <QTreeWidget>

class MBExperimentListWidgetPrivate;
class MBExperimentListWidget : public QTreeWidget
{
    Q_OBJECT
public:
    friend class MBExperimentListWidgetPrivate;
    MBExperimentListWidget();
    virtual ~MBExperimentListWidget();

    void setExperimentManager(MBExperimentManager *EM);
    void refresh();
signals:
    void experimentActivated(QString id);
private slots:
    void slot_item_activated(QTreeWidgetItem *item);
private:
    MBExperimentListWidgetPrivate *d;
};

#endif // MBEXPERIMENTLIST_H

