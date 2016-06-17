/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef TABBER_H
#define TABBER_H

#include <QTabWidget>
#include <QString>

/**
 *  \class Tabber
 *  \brief Use this class to control two or more tab widgets so that the user
 * may move widgets from one to the other. For now user double clicks on a tab
 * to move it to the other, but in the future, drag/drop should be enabled. Also
 * we'd like user to be able to drag it off the widget to become undocked (floating).
 */

class TabberTabWidget;
class TabberPrivate;
class Tabber : public QObject {
    Q_OBJECT
public:
    friend class TabberPrivate;
    Tabber();
    virtual ~Tabber();
    TabberTabWidget* createTabWidget(const QString& container_name);
    void addWidget(const QString& container_name, const QString& label, QWidget* W);
    void addWidget(TabberTabWidget* TW, const QString& label, QWidget* W);
    QString currentContainerName();
    void setCurrentContainerName(const QString& container_name);
    void setCurrentContainer(TabberTabWidget* TW);
    void switchCurrentContainer();
    QList<QWidget*> allWidgets();

    void moveWidgetToOtherContainer(QWidget* W);
    void popOutWidget(QWidget* W);
private slots:
    void slot_tab_close_requested(int index);
    void slot_tab_bar_clicked(int index);
    void slot_tab_bar_double_clicked(int index);
    void slot_widget_destroyed(QObject* obj);
    void slot_recalculate_suggested_changed();

private:
    TabberPrivate* d;
};

class TabberTabWidgetPrivate;
class TabberTabWidget : public QTabWidget {
    Q_OBJECT
public:
    friend class TabberTabWidgetPrivate;
    TabberTabWidget();
    virtual ~TabberTabWidget();

private:
    TabberTabWidgetPrivate* d;
};

#endif // TABBER_H
