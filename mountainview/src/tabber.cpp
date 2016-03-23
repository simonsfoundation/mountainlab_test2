#include "tabber.h"
#include <QMap>
#include <QVariant>
#include <QDebug>

struct TabberWidget {
	QWidget *widget;
	QString label;
	QString current_container_name;
};

class TabberPrivate {
public:
	Tabber *q;
	QMap<QString,QTabWidget *> m_tab_widgets;
	QList<TabberWidget> m_widgets;

	void put_widget_in_container(QString container_name,QWidget *W);
	TabberWidget *find_tabber_widget(QWidget *W);
	int find_widget_index_in_container(QString container_name,QWidget *W);
	void remove_widgets_that_are_no_longer_in_their_containers();
};

Tabber::Tabber()
{
	d=new TabberPrivate;
	d->q=this;
}

Tabber::~Tabber()
{
	delete d;
}

void Tabber::addTabWidget(const QString &container_name, QTabWidget *TW)
{
	d->m_tab_widgets[container_name]=TW;
	TW->setTabsClosable(true);
	TW->setMovable(true);
	TW->setProperty("container_name",container_name);
	connect(TW,SIGNAL(tabCloseRequested(int)),this,SLOT(slot_tab_close_requested(int)),Qt::DirectConnection); //important to do direct connection so we intercept before the tab is actually removed
}


void Tabber::addWidget(const QString &container_name, const QString &label, QWidget *W)
{
	TabberWidget X;
	X.widget=W;
	X.label=label;
	d->m_widgets << X;
	d->put_widget_in_container(container_name,X.widget);
}

void Tabber::addWidget(QTabWidget *TW, const QString &label, QWidget *W)
{
	QString container_name=TW->property("container_name").toString();
	if (container_name.isEmpty()) return;
	addWidget(container_name,label,W);
}

QList<QWidget *> Tabber::allWidgets()
{
	QList<QWidget *> ret;
	for (int i=0; i<d->m_widgets.count(); i++) {
		ret << d->m_widgets[i].widget;
	}
	return ret;
}

void Tabber::slot_tab_close_requested(int index)
{
	Q_UNUSED(index)
	d->remove_widgets_that_are_no_longer_in_their_containers();
}

void TabberPrivate::put_widget_in_container(QString container_name,QWidget *W)
{
	TabberWidget *X=find_tabber_widget(W);
	if (!X) return;
	if (!container_name.isEmpty()) {
		if (!m_tab_widgets.contains(container_name)) return;
		int index=m_tab_widgets[container_name]->addTab(X->widget,X->label);
		if (index>=0) {
			m_tab_widgets[container_name]->setCurrentIndex(index);
		}
	}
	else {
		if (!X->current_container_name.isEmpty()) {
			//fix this.... we need to put the widget into a new floating container!
			int index=find_widget_index_in_container(X->current_container_name,X->widget);
			if (index<0) return;
			m_tab_widgets[X->current_container_name]->removeTab(index);
		}
	}
	X->current_container_name=container_name;
}

TabberWidget *TabberPrivate::find_tabber_widget(QWidget *W)
{
	for (int i=0; i<m_widgets.count(); i++) {
		if (m_widgets[i].widget==W) {
			return &m_widgets[i];
		}
	}
	return 0;
}

int TabberPrivate::find_widget_index_in_container(QString container_name, QWidget *W)
{
	if (!m_tab_widgets.contains(container_name)) return -1;
	QTabWidget *TW=m_tab_widgets[container_name];
	for (int i=0; i<TW->count(); i++) {
		if (TW->widget(i)==W) return i;
	}
	return -1;
}

void TabberPrivate::remove_widgets_that_are_no_longer_in_their_containers()
{
	for (int i=0; i<m_widgets.count(); i++) {
		QString cname=m_widgets[i].current_container_name;
		if (!cname.isEmpty()) {
			int index=find_widget_index_in_container(cname,m_widgets[i].widget);
			if (index<0) {
				m_widgets.removeAt(index);
				i--;
			}
		}
	}
}
