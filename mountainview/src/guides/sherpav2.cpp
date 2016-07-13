/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 7/5/2016
*******************************************************/

#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <flowlayout.h>
#include <QLineEdit>
#include <mountainprocessrunner.h>
#include <taskprogress.h>
#include <QThread>
#include "sherpav2.h"

#include "mv_compute_templates.h"
#include "individualmergedecisionpage.h"

class SherpaV2Private {
public:
    SherpaV2* q;

    MVContext* m_context;
    MVMainWindow* m_main_window;

    QWizardPage* make_page_1();
    QWizardPage* make_page_2();
    QWizardPage* make_page_3();
    QWizardPage* make_page_4();
    QWizardPage* make_page_5();

    QAbstractButton* make_instructions_button(QString text, QString instructions);
    QAbstractButton* make_open_view_button(QString text, QString view_id, QString container_name);

    void show_instructions(QString title, QString instructions);
};

SherpaV2::SherpaV2(MVContext* mvcontext, MVMainWindow* mw)
{
    d = new SherpaV2Private;
    d->q = this;
    d->m_context = mvcontext;
    d->m_main_window = mw;

    this->addPage(d->make_page_1());
    this->addPage(d->make_page_2());
    this->addPage(d->make_page_3());
    this->addPage(d->make_page_4());
    this->addPage(d->make_page_5());
    this->addPage(new IndividualMergeDecisionPage(mvcontext, mw));

    this->resize(800, 600);

    this->setWindowTitle("Sherpa Version 2");
}

SherpaV2::~SherpaV2()
{
    delete d;
}

void SherpaV2::slot_button_clicked()
{
    QString action = sender()->property("action").toString();
    if (action == "open_view") {
        d->m_main_window->setCurrentContainerName(sender()->property("container-name").toString());
        d->m_main_window->openView(sender()->property("view-id").toString());
    }
    else if (action == "show_instructions") {
        d->show_instructions(sender()->property("title").toString(), sender()->property("instructions").toString());
    }
}

void SherpaV2::slot_select_merge_candidates()
{

    QSet<ClusterPair> pairs;
    QList<ClusterPair> keys = d->m_context->clusterPairAttributesKeys();
    foreach (ClusterPair key, keys) {
        if (d->m_context->clusterPairTags(key).contains("merge_candidate")) {
            pairs.insert(key);
        }
    }
    d->m_context->setSelectedClusterPairs(pairs);
}

QWizardPage* SherpaV2Private::make_page_1()
{
    QWizardPage* page = new QWizardPage;
    page->setTitle("Cluster detail view");

    QLabel* label = new QLabel(TextFile::read(":/guides/sherpav2/page_cluster_details.txt"));
    label->setWordWrap(true);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(label);
    page->setLayout(layout);

    FlowLayout* flayout = new FlowLayout;
    flayout->addWidget(make_open_view_button("Cluster details", "open-cluster-details", "north"));
    layout->addLayout(flayout);

    return page;
}

QWizardPage* SherpaV2Private::make_page_2()
{
    QWizardPage* page = new QWizardPage;
    page->setTitle("Event Filter");

    QLabel* label = new QLabel(TextFile::read(":/guides/sherpav2/page_event_filter.txt"));
    label->setWordWrap(true);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(label);
    page->setLayout(layout);

    //FlowLayout* flayout = new FlowLayout;
    //flayout->addWidget(make_open_view_button("Cluster details", "open-cluster-details", "north"));
    //layout->addLayout(flayout);

    return page;
}

QWizardPage* SherpaV2Private::make_page_3()
{
    QWizardPage* page = new QWizardPage;
    page->setTitle("Cluster detail view");

    QLabel* label = new QLabel(TextFile::read(":/guides/sherpav2/page_auto_correlograms.txt"));
    label->setWordWrap(true);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(label);
    page->setLayout(layout);

    FlowLayout* flayout = new FlowLayout;
    flayout->addWidget(make_open_view_button("Auto-correlograms", "open-auto-correlograms", "south"));
    layout->addLayout(flayout);

    return page;
}

QWizardPage* SherpaV2Private::make_page_4()
{
    QWizardPage* page = new QWizardPage;
    page->setTitle("Discrimination histograms");

    QLabel* label = new QLabel(TextFile::read(":/guides/sherpav2/page_discrim_hist.txt"));
    label->setWordWrap(true);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(label);
    page->setLayout(layout);

    FlowLayout* flayout = new FlowLayout;
    flayout->addWidget(make_open_view_button("Discrimination histograms", "open-discrim-histograms-sherpa", "south"));
    layout->addLayout(flayout);

    return page;
}

QWizardPage* SherpaV2Private::make_page_5()
{
    QWizardPage* page = new QWizardPage;
    page->setTitle("Cross-correlograms");

    QLabel* label = new QLabel(TextFile::read(":/guides/sherpav2/page_cross_correlograms.txt"));
    label->setWordWrap(true);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(label);
    page->setLayout(layout);

    FlowLayout* flayout = new FlowLayout;
    {
        QPushButton* B = new QPushButton("Select merge candidates");
        QObject::connect(B, SIGNAL(clicked(bool)), q, SLOT(slot_select_merge_candidates()));
        flayout->addWidget(B);
    }
    flayout->addWidget(make_open_view_button("Cross-correlograms", "open-selected-cross-correlograms", "south"));
    layout->addLayout(flayout);

    return page;
}

QAbstractButton* SherpaV2Private::make_instructions_button(QString text, QString instructions)
{
    QPushButton* B = new QPushButton(text);
    B->setProperty("action", "show_instructions");
    B->setProperty("instructions", instructions);
    QObject::connect(B, SIGNAL(clicked(bool)), q, SLOT(slot_button_clicked()));
    return B;
}

QAbstractButton* SherpaV2Private::make_open_view_button(QString text, QString view_id, QString container_name)
{
    QPushButton* B = new QPushButton(text);
    B->setProperty("action", "open_view");
    B->setProperty("view-id", view_id);
    B->setProperty("container-name", container_name);
    QObject::connect(B, SIGNAL(clicked(bool)), q, SLOT(slot_button_clicked()));
    return B;
}

void SherpaV2Private::show_instructions(QString title, QString instructions)
{
    QMessageBox::information(q, title, instructions);
}
