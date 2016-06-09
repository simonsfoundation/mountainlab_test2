/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef MVOVERVIEW2WIDGETCONTROLPANEL_H
#define MVOVERVIEW2WIDGETCONTROLPANEL_H

#include <QScrollArea>
#include <QVariant>
#include <QWidget>

/** \class MVOverview2WidgetControlPanel
 *  \brief The main control panel of MVOverview2Widget where the user can request to open views and change settings
 */

class MVOverview2WidgetControlPanelPrivate;
class MVOverview2WidgetControlPanel : public QWidget {
    Q_OBJECT
public:
    friend class MVOverview2WidgetControlPanelPrivate;
    MVOverview2WidgetControlPanel(QWidget* parent = 0);
    virtual ~MVOverview2WidgetControlPanel();

    ///Get parameter value for a particular control defined internally
    QVariant getParameterValue(QString name, const QVariant& defaultval = QVariant());
    ///Set parameter value for a particular control defined internally
    void setParameterValue(QString name, QVariant val);
    ///Corresponds to certain types of controls
    void setParameterLabel(QString name, QString text);
    ///Corresponds to certain types of controls
    void setParameterChoices(QString name, QStringList choices);
signals:
    ///A button has clicked.... the name of the button is in str
    void signalButtonClicked(QString str);
    ///A combo box has been activated.... the name of the combobox is in str
    void signalComboBoxActivated(QString str);
private
slots:
    void slot_button_clicked();
    void slot_checkbox_clicked(bool val);
    void slot_radio_button_clicked();
    void slot_combobox_activated();

private:
    MVOverview2WidgetControlPanelPrivate* d;
};

#endif // MVOVERVIEW2WIDGETCONTROLPANEL_H
