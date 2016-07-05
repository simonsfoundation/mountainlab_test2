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
#include "sherpav1.h"
#include "textfile.h"

class SherpaV1Private {
public:
    SherpaV1* q;

    MVContext* m_context;
    MVMainWindow* m_main_window;

    QWizardPage* make_page_1();
    QWizardPage* make_page_2();
    QWizardPage* make_page_3();

    QAbstractButton* make_instructions_button(QString text, QString instructions);
    QAbstractButton* make_open_view_button(QString text, QString view_id, QString container_name);

    void show_instructions(QString title, QString instructions);
};

SherpaV1::SherpaV1(MVContext* mvcontext, MVMainWindow* mw)
{
    d = new SherpaV1Private;
    d->q = this;
    d->m_context = mvcontext;
    d->m_main_window = mw;

    this->addPage(d->make_page_1());
    this->addPage(d->make_page_2());
    this->addPage(d->make_page_3());

    this->resize(500, 600);

    this->setWindowTitle("Sherpa Version 1");
}

SherpaV1::~SherpaV1()
{
    delete d;
}

void SherpaV1::slot_button_clicked()
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

QWizardPage* SherpaV1Private::make_page_1()
{
    QWizardPage* page = new QWizardPage;
    page->setTitle("Cluster detail view");

    QLabel* label = new QLabel(read_text_file(":/guides/sherpav1/page_1.txt"));
    label->setWordWrap(true);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(label);
    page->setLayout(layout);

    FlowLayout* flayout = new FlowLayout;
    flayout->addWidget(make_open_view_button("Cluster details", "open-cluster-details", "north"));
    layout->addLayout(flayout);

    return page;
}

QWizardPage* SherpaV1Private::make_page_2()
{
    QWizardPage* page = new QWizardPage;
    page->setTitle("Event Filter");

    QLabel* label = new QLabel(read_text_file(":/guides/sherpav1/page_2.txt"));
    label->setWordWrap(true);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(label);
    page->setLayout(layout);

    //FlowLayout* flayout = new FlowLayout;
    //flayout->addWidget(make_open_view_button("Cluster details", "open-cluster-details", "north"));
    //layout->addLayout(flayout);

    return page;
}

QWizardPage* SherpaV1Private::make_page_3()
{
    return new QWizardPage;
}

QAbstractButton* SherpaV1Private::make_instructions_button(QString text, QString instructions)
{
    QPushButton* B = new QPushButton(text);
    B->setProperty("action", "show_instructions");
    B->setProperty("instructions", instructions);
    QObject::connect(B, SIGNAL(clicked(bool)), q, SLOT(slot_button_clicked()));
    return B;
}

QAbstractButton* SherpaV1Private::make_open_view_button(QString text, QString view_id, QString container_name)
{
    QPushButton* B = new QPushButton(text);
    B->setProperty("action", "open_view");
    B->setProperty("view-id", view_id);
    B->setProperty("container-name",container_name);
    QObject::connect(B, SIGNAL(clicked(bool)), q, SLOT(slot_button_clicked()));
    return B;
}

void SherpaV1Private::show_instructions(QString title, QString instructions)
{
    QMessageBox::information(q, title, instructions);
}
