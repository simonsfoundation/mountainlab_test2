/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/17/2016
*******************************************************/

#ifndef MVCONTROLPANEL_H
#define MVCONTROLPANEL_H

#include <QAbstractButton>
#include <QJsonObject>
#include <QString>
#include <QWidget>

struct MVViewOptions {
    MVViewOptions()
    {
        cc_max_dt_msec = 100;
        clip_size = 100;
    }

    QString timeseries;
    double cc_max_dt_msec;
    int clip_size;
    static MVViewOptions fromJsonObject(QJsonObject obj);
    QJsonObject toJsonObject() const;
};

struct MVEventFilter {
    MVEventFilter()
    {
        use_shell_split = false;
        shell_increment = 2;
        min_per_shell = 150;
        use_event_filter = false;
        min_detectability_score = 0;
        max_outlier_score = 0;
    }

    bool use_shell_split;
    double shell_increment;
    int min_per_shell;
    bool use_event_filter;
    double min_detectability_score;
    double max_outlier_score;
    static MVEventFilter fromJsonObject(QJsonObject obj);
    QJsonObject toJsonObject() const;
};

class MVControlPanelPrivate;
class MVControlPanel : public QWidget {
    Q_OBJECT
public:
    friend class MVControlPanelPrivate;
    MVControlPanel();
    virtual ~MVControlPanel();

    void setTimeseriesChoices(const QStringList& names);

    MVViewOptions viewOptions() const;
    MVEventFilter eventFilter() const;

    void setViewOptions(MVViewOptions opts);
    void setEventFilter(MVEventFilter X);

    QAbstractButton* findButton(const QString& name);

signals:
    void userAction(QString name);

private
slots:
    void slot_update_enabled_controls();
    void slot_button_clicked();

private:
    MVControlPanelPrivate* d;
};

#endif // MVCONTROLPANEL_H
