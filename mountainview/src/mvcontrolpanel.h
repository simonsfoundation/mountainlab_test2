/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/17/2016
*******************************************************/

#ifndef MVCONTROLPANEL_H
#define MVCONTROLPANEL_H

#include "mvviewagent.h"

#include <QAbstractButton>
#include <QJsonObject>
#include <QString>
#include <QWidget>

class MVControlPanelPrivate;
class MVControlPanel : public QWidget {
    Q_OBJECT
public:
    friend class MVControlPanelPrivate;
    MVControlPanel(MVViewAgent* view_agent);
    virtual ~MVControlPanel();

    void setTimeseriesChoices(const QStringList& names);

    //MVEventFilter eventFilter() const;
    //void setEventFilter(MVEventFilter X);

    QAbstractButton* findButton(const QString& name);

signals:
    void userAction(QString name);

private
slots:
    void slot_update_enabled_controls();
    void slot_button_clicked();
    void slot_view_agent_option_changed(QString name);
    void slot_view_agent_event_filter_changed();
    void slot_update_timeseries_box();

private:
    MVControlPanelPrivate* d;
};

#endif // MVCONTROLPANEL_H
