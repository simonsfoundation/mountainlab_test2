#ifndef MVOVERVIEW2WIDGETCONTROLPANEL_H
#define MVOVERVIEW2WIDGETCONTROLPANEL_H

#include <QWidget>


class MVOverview2WidgetControlPanelPrivate;
class MVOverview2WidgetControlPanel : public QWidget
{
	Q_OBJECT
public:
	friend class MVOverview2WidgetControlPanelPrivate;
	MVOverview2WidgetControlPanel(QWidget *parent=0);
	virtual ~MVOverview2WidgetControlPanel();

	QVariant getParameterValue(QString name);
    void setParameterValue(QString name,QVariant val);
    void setParameterLabel(QString name,QString text);
    void setParameterChoices(QString name,QStringList choices);
signals:
	void signalButtonClicked(QString str);
    void signalComboBoxActivated(QString str);
private slots:
	void slot_button_clicked();
    void slot_checkbox_clicked(bool val);
	void slot_radio_button_clicked();
    void slot_combobox_activated();
private:
	MVOverview2WidgetControlPanelPrivate *d;
};

#endif // MVOVERVIEW2WIDGETCONTROLPANEL_H
