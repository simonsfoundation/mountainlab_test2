/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 9/29/2016
*******************************************************/

#include "prvguiitemdetailwidget.h"
#include "prvguimaincontrolwidget.h"
#include "prvguimainwindow.h"
#include "prvguitreewidget.h"

#include <QFileInfo>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QMutex>
#include <QProcess>
#include <QSplitter>
#include <QTreeWidget>
#include <taskprogress.h>
#include "taskprogressview.h"
#include "prvguicontrolpanel.h"

class PrvGuiMainWindowPrivate {
public:
    PrvGuiMainWindow* q;

    PrvGuiTreeWidget* m_tree;
    PrvGuiControlPanel* m_control_panel;

    QSplitter* m_splitter;
    QSplitter* m_left_splitter;

    void update_sizes();
};

PrvGuiMainWindow::PrvGuiMainWindow()
{
    d = new PrvGuiMainWindowPrivate;
    d->q = this;

    TaskProgressView* TPV = new TaskProgressView; //seems important to create this first

    d->m_tree = new PrvGuiTreeWidget;

    d->m_control_panel = new PrvGuiControlPanel;
    d->m_control_panel->addControlWidget("Main", new PrvGuiMainControlWidget(d->m_tree));
    d->m_control_panel->addControlWidget("Item details", new PrvGuiItemDetailWidget(d->m_tree));

    d->m_left_splitter = new QSplitter(Qt::Vertical);
    d->m_left_splitter->addWidget(d->m_control_panel);
    d->m_left_splitter->addWidget(TPV);

    d->m_splitter = new QSplitter(Qt::Horizontal);
    d->m_splitter->addWidget(d->m_left_splitter);
    d->m_splitter->addWidget(d->m_tree);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(d->m_splitter);
    this->setLayout(layout);

    d->m_tree->refresh();
    d->update_sizes();
}

PrvGuiMainWindow::~PrvGuiMainWindow()
{
    delete d;
}

void PrvGuiMainWindow::setPrvs(const QList<PrvRecord>& prvs)
{
    d->m_tree->setPrvs(prvs);
}

void PrvGuiMainWindow::setServerNames(QStringList names)
{
    d->m_tree->setServerNames(names);
}

void PrvGuiMainWindow::refresh()
{
    d->m_tree->refresh();
}

void PrvGuiMainWindow::resizeEvent(QResizeEvent* evt)
{
    QWidget::resizeEvent(evt);
    d->update_sizes();
}

void PrvGuiMainWindowPrivate::update_sizes()
{
    float W0 = q->width();
    float H0 = q->height();

    int W1 = W0 / 3.5;
    if (W1 < 150)
        W1 = 150;
    if (W1 > 800)
        W1 = 800;
    int W2 = W0 - W1;

    int H1 = H0 / 2;
    int H2 = H0 / 2;

    {
        QList<int> sizes;
        sizes << W1 << W2;
        m_splitter->setSizes(sizes);
    }
    {
        QList<int> sizes;
        sizes << H1 << H2;
        m_left_splitter->setSizes(sizes);
    }
}
