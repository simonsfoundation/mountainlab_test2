#include "clusterannotationguide.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include "flowlayout.h"

class ClusterAnnotationGuidePrivate {
public:
    ClusterAnnotationGuide* q;
    MVMainWindow* m_main;
    MVContext* m_context;

    QWizardPage* make_intro_page();
    QWizardPage* make_noise_page();

    QAbstractButton* make_instructions_button(QString text, QString instructions);
    QAbstractButton* make_user_action_button(QString text, QString action);
};

ClusterAnnotationGuide::ClusterAnnotationGuide(MVContext* mvcontext, MVMainWindow* X)
{
    d = new ClusterAnnotationGuidePrivate;
    d->q = this;
    d->m_main = X;
    d->m_context = mvcontext;

    connect(X, SIGNAL(destroyed(QObject*)), this, SLOT(deleteLater()));

    {
        QWizardPage* page = d->make_intro_page();
        this->addPage(page);
    }
    {
        QWizardPage* page = d->make_noise_page();
        this->addPage(page);
    }

    this->resize(500, 600);
}

ClusterAnnotationGuide::~ClusterAnnotationGuide()
{
    delete d;
}

void ClusterAnnotationGuide::slot_user_action_button_clicked()
{
    QString action = sender()->property("user_action").toString();
    d->m_main->show();
    d->m_main->applyUserAction(action);
}

void ClusterAnnotationGuide::slot_instructions_button_clicked()
{
    QString instructions = sender()->property("instructions").toString();
    QMessageBox::information(this, "Instructions", instructions);
}

QWizardPage* ClusterAnnotationGuidePrivate::make_intro_page()
{
    QWizardPage* page = new QWizardPage;
    page->setTitle("Cluster Annotation Guide");

    QLabel* label = new QLabel("This is the cluster annotation guide. It is under development.");
    label->setWordWrap(true);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(label);
    page->setLayout(layout);

    return page;
}

QWizardPage* ClusterAnnotationGuidePrivate::make_noise_page()
{
    QWizardPage* page = new QWizardPage;
    page->setTitle("Reject clusters too close to noise");

    QLabel* label = new QLabel("Purpose: to reject clusters that are too close to the noise. "
                               "This may includes those that are pure noise or those that include spikes from multiple "
                               "low-amplitude units.");
    label->setWordWrap(true);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(label);

    QString instructions = "Open the [Amplitude Histograms] view. Select all clusters that appear to be "
                           "significantly cut off by the threshold. Then press [T] and choose the 'reject' tag."
                           "You may also want to select other descriptive tags for individual clusters.";

    FlowLayout* flayout = new FlowLayout;
    flayout->addWidget(make_instructions_button("Instructions", instructions));
    flayout->addWidget(make_user_action_button("Amplitude Histograms", "open-amplitude-histograms"));
    layout->addLayout(flayout);

    page->setLayout(layout);

    return page;
}

QAbstractButton* ClusterAnnotationGuidePrivate::make_instructions_button(QString text, QString instructions)
{
    QPushButton* B = new QPushButton(text);
    B->setProperty("instructions", instructions);
    QObject::connect(B, SIGNAL(clicked(bool)), q, SLOT(slot_instructions_button_clicked()));
    return B;
}

QAbstractButton* ClusterAnnotationGuidePrivate::make_user_action_button(QString text, QString action)
{
    QPushButton* B = new QPushButton(text);
    B->setProperty("user_action", action);
    QObject::connect(B, SIGNAL(clicked(bool)), q, SLOT(slot_user_action_button_clicked()));
    return B;
}
