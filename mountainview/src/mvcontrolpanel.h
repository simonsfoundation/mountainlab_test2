/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/17/2016
*******************************************************/

#ifndef MVCONTROLPANEL_H
#define MVCONTROLPANEL_H

#include "flowlayout.h"
#include "mvviewagent.h"

#include <QAbstractButton>
#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QJsonObject>
#include <QPushButton>
#include <QRadioButton>
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

    QLayout *viewLayout() const;

signals:
    void userAction(QString name);

private
slots:
    void slot_update_enabled_controls();
    void slot_button_clicked();
    void slot_view_agent_option_changed(QString name);
    void slot_view_agent_event_filter_changed();
    void slot_update_timeseries_box();
    void slot_control_changed();
    void slot_update_view_agent();

private:
    MVControlPanelPrivate* d;
};

class ControlManager : public QObject {
    Q_OBJECT
public:
    void add_group_label(QGridLayout* G, QString label);
    QCheckBox* add_check_box(QGridLayout* G, QString name, QString label, bool val);
    QComboBox* add_combo_box(QGridLayout* G, QString name, QString label);
    QLineEdit* add_int_box(QGridLayout* G, QString name, QString label, int val, int minval, int maxval);
    QLineEdit* add_float_box(QGridLayout* G, QString name, QString label, float val, float minval, float maxval);
    QGroupBox* add_radio_button_group(QGridLayout* G, QString name, QStringList options, QString val);
    QPushButton* add_button(QGridLayout* G, QString name, QString label);
    void add_horizontal_divider_line(QVBoxLayout* layout);

    QVariant get_parameter_value(QString name, const QVariant& defaultval = QVariant());
    void set_parameter_value(QString name, QVariant val);
    void set_parameter_label(QString name, QString text);
    void set_parameter_choices(QString name, QStringList choices);
    void set_parameter_enabled(QString name, bool val);

    QCheckBox* checkbox(QString name);

signals:
    void controlChanged();

private:
    QMap<QString, QLineEdit*> m_lineedit_controls;
    QMap<QString, QCheckBox*> m_checkbox_controls;
    QMap<QString, QGroupBox*> m_groupbox_controls;
    QMap<QString, QComboBox*> m_combobox_controls;
    QMap<QString, QPushButton*> m_buttons;
};

class ClusterVisibilityControls : public QObject {
    Q_OBJECT
public:
    ClusterVisibilityControls(MVContext* mvcontext, FlowLayout* layout);
    virtual ~ClusterVisibilityControls();

private:
    MVContext* m_context;
    FlowLayout* m_flayout;
    QList<QWidget*> m_controls;

    QRadioButton* add_control(QString tag, QString label);
private
slots:
    void slot_controls_changed();
    void slot_update_controls();
};

#endif // MVCONTROLPANEL_H
