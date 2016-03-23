#include "tabber.h"
#include <QMap>
#include <QVariant>
#include <QDebug>

struct TabberWidget {
    QWidget* widget;
    QString label;
    QString current_container_name;
};

class TabberPrivate {
public:
    Tabber* q;
    QMap<QString, TabberTabWidget*> m_tab_widgets;
    QList<TabberWidget> m_widgets;
    QString m_current_container_name;

    void put_widget_in_container(QString container_name, QWidget* W);
    TabberWidget* find_tabber_widget(QWidget* W);
    int find_widget_index_in_container(QString container_name, QWidget* W);
    void remove_widget(QWidget* W);
    QString find_other_container_name(QString name);
};

Tabber::Tabber()
{
    d = new TabberPrivate;
    d->q = this;
}

Tabber::~Tabber()
{
    delete d;
}

TabberTabWidget* Tabber::createTabWidget(const QString& container_name)
{
    TabberTabWidget* TW = new TabberTabWidget;
    d->m_tab_widgets[container_name] = TW;
    TW->setProperty("container_name", container_name);
    connect(TW, SIGNAL(tabCloseRequested(int)), this, SLOT(slot_tab_close_requested(int)));
    connect(TW, SIGNAL(tabBarClicked(int)), this, SLOT(slot_tab_bar_clicked(int)));
    connect(TW, SIGNAL(tabBarDoubleClicked(int)), this, SLOT(slot_tab_bar_double_clicked(int)));
    d->m_current_container_name = container_name;
    return TW;
}

void Tabber::addWidget(const QString& container_name, const QString& label, QWidget* W)
{
    TabberWidget X;
    X.widget = W;
    X.label = label;
    d->m_widgets << X;
    d->put_widget_in_container(container_name, X.widget);
}

void Tabber::addWidget(TabberTabWidget* TW, const QString& label, QWidget* W)
{
    QString container_name = TW->property("container_name").toString();
    if (container_name.isEmpty())
        return;
    addWidget(container_name, label, W);
}

QString Tabber::currentContainerName()
{
    return d->m_current_container_name;
}

void Tabber::setCurrentContainerName(const QString& container_name)
{
    d->m_current_container_name = container_name;
}

void Tabber::switchCurrentContainer()
{
    d->m_current_container_name = d->find_other_container_name(d->m_current_container_name);
}

QList<QWidget*> Tabber::allWidgets()
{
    QList<QWidget*> ret;
    for (int i = 0; i < d->m_widgets.count(); i++) {
        ret << d->m_widgets[i].widget;
    }
    return ret;
}

void Tabber::slot_tab_close_requested(int index)
{
    qDebug() << __FUNCTION__ << __LINE__;
    Q_UNUSED(index)
    TabberTabWidget* TW = (TabberTabWidget*)sender();
    QWidget* W = TW->widget(index);
    d->remove_widget(W);
    delete W;
}

void Tabber::slot_tab_bar_clicked(int index)
{
    Q_UNUSED(index);
    TabberTabWidget* TW = (TabberTabWidget*)sender();
    d->m_current_container_name = TW->property("container_name").toString();
}

void Tabber::slot_tab_bar_double_clicked(int index)
{
    TabberTabWidget* TW = (TabberTabWidget*)sender();
    QWidget* W = TW->widget(index);
    if (!W)
        return;
    TabberWidget* X = d->find_tabber_widget(W);
    if (!X)
        return;
    QString cname = X->current_container_name;
    QString cname2 = d->find_other_container_name(cname);
    d->put_widget_in_container(cname2, W);
}

void TabberPrivate::put_widget_in_container(QString container_name, QWidget* W)
{
    TabberWidget* X = find_tabber_widget(W);
    if (!X)
        return;
    if (!container_name.isEmpty()) {
        if (!m_tab_widgets.contains(container_name))
            return;
        int index = m_tab_widgets[container_name]->addTab(X->widget, X->label);
        if (index >= 0) {
            m_tab_widgets[container_name]->setCurrentIndex(index);
        }
    } else {
        if (!X->current_container_name.isEmpty()) {
            //fix this.... we need to put the widget into a new floating container!
            int index = find_widget_index_in_container(X->current_container_name, X->widget);
            if (index < 0)
                return;
            m_tab_widgets[X->current_container_name]->removeTab(index);
        }
    }
    X->current_container_name = container_name;
    m_current_container_name = container_name;
}

TabberWidget* TabberPrivate::find_tabber_widget(QWidget* W)
{
    for (int i = 0; i < m_widgets.count(); i++) {
        if (m_widgets[i].widget == W) {
            return &m_widgets[i];
        }
    }
    return 0;
}

int TabberPrivate::find_widget_index_in_container(QString container_name, QWidget* W)
{
    if (!m_tab_widgets.contains(container_name))
        return -1;
    TabberTabWidget* TW = m_tab_widgets[container_name];
    for (int i = 0; i < TW->count(); i++) {
        if (TW->widget(i) == W)
            return i;
    }
    return -1;
}

void TabberPrivate::remove_widget(QWidget* W)
{
    for (int i = 0; i < m_widgets.count(); i++) {
        if (m_widgets[i].widget == W) {
            m_widgets.removeAt(i);
            return;
        }
    }
}

QString TabberPrivate::find_other_container_name(QString name)
{
    QStringList keys = m_tab_widgets.keys();
    foreach(QString str, keys)
    {
        if (str != name)
            return str;
    }
    return "";
}

class TabberTabWidgetPrivate {
public:
    TabberTabWidget* q;
};

TabberTabWidget::TabberTabWidget()
    : QTabWidget()
{
    d = new TabberTabWidgetPrivate;
    d->q = this;
    this->setTabsClosable(true);
    this->setMovable(true);
}

TabberTabWidget::~TabberTabWidget()
{
    delete d;
}
