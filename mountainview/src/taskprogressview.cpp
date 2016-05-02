/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/30/2016
*******************************************************/

#include "taskprogressview.h"
#include "taskprogress.h"
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QLabel>

class TaskProgressViewPrivate {
public:
    TaskProgressView* q;
    TaskProgressAgent* m_agent;
    QTreeWidget* m_tree;

    QString shortened(QString txt, int maxlen);
};

TaskProgressView::TaskProgressView()
{
    d = new TaskProgressViewPrivate;
    d->q = this;
    d->m_agent = TaskProgressAgent::globalInstance();

    d->m_tree = new QTreeWidget;
    d->m_tree->header()->hide();

    QVBoxLayout* vlayout = new QVBoxLayout;
    vlayout->addWidget(d->m_tree);
    vlayout->setMargin(0);
    vlayout->setSpacing(0);
    this->setLayout(vlayout);

    connect(d->m_agent, SIGNAL(tasksChanged()), this, SLOT(slot_refresh()));
}

TaskProgressView::~TaskProgressView()
{
    delete d;
}

void TaskProgressView::slot_refresh()
{
    QTreeWidget* T = d->m_tree;
    TaskProgressAgent* A = d->m_agent;

    T->clear();
    QStringList labels;
    labels << "Task";
    T->setHeaderLabels(labels);

    QList<TaskInfo> tasks1 = A->activeTasks();
    QList<TaskInfo> tasks2 = A->completedTasks();
    QList<TaskInfo> tasks;
    tasks.append(tasks1);
    tasks.append(tasks2);
    for (int i=0; i<tasks.count(); i++) {
        TaskInfo info=tasks[i];
        QTreeWidgetItem* it = new QTreeWidgetItem;
        QString txt;
        QString col;
        if (i<tasks1.count()) { //active
            txt = QString("%1 (%2%) %3").arg(info.label).arg((int)(info.progress * 100)).arg(info.description);
            col = "blue";
        }
        else {
            txt = QString("%1 (%2 sec) %3").arg(info.label).arg(info.start_time.msecsTo(info.end_time) / 1000.0).arg(info.description);
            col = "black";
        }
        QLabel* label = new QLabel(d->shortened(txt, 150));

        label->setStyleSheet(QString("QLabel { color: %1 }").arg(col));

        it->setToolTip(0, txt);
        T->addTopLevelItem(it);
        T->setItemWidget(it, 0, label);
    }
    for (int i = 0; i < T->columnCount(); i++) {
        T->resizeColumnToContents(i);
    }
}

QString TaskProgressViewPrivate::shortened(QString txt, int maxlen)
{
    if (txt.count() > maxlen) {
        return txt.mid(0, maxlen - 3) + "...";
    }
    else
        return txt;
}
