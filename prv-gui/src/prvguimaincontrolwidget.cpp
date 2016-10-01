/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 10/1/2016
*******************************************************/

#include "prvguidownloaddialog.h"
#include "prvguimaincontrolwidget.h"
#include "prvguiuploaddialog.h"
#include <QGridLayout>
#include <QPushButton>
#include <QDebug>
#include <taskprogress.h>
#include "prvupload.h"
#include "prvdownload.h"

class PrvGuiMainControlWidgetPrivate {
public:
    PrvGuiMainControlWidget* q;
    PrvGuiTreeWidget* m_tree;
    QList<QAbstractButton*> m_all_buttons;

    void update_enabled();
    QAbstractButton* find_button(QString name);
    QPushButton* create_push_button(QString name, QString label);
};

PrvGuiMainControlWidget::PrvGuiMainControlWidget(PrvGuiTreeWidget* TW)
{
    d = new PrvGuiMainControlWidgetPrivate;
    d->q = this;

    QGridLayout* grid_layout = new QGridLayout;
    this->setLayout(grid_layout);
    int row = 0;
    {
        QPushButton* button = d->create_push_button("search_again", "Search again");
        QObject::connect(button, SIGNAL(clicked(bool)), this, SLOT(slot_refresh_tree()));
        grid_layout->addWidget(button, row, 0, 1, 2);
    }
    row++;
    {
        QPushButton* button = d->create_push_button("upload", "Upload...");
        QObject::connect(button, SIGNAL(clicked(bool)), this, SLOT(slot_upload()));
        grid_layout->addWidget(button, row, 0, 1, 1);
    }
    {
        QPushButton* button = d->create_push_button("download", "Download...");
        QObject::connect(button, SIGNAL(clicked(bool)), this, SLOT(slot_download()));
        grid_layout->addWidget(button, row, 1, 1, 1);
    }
    row++;

    d->m_tree = TW;
    QObject::connect(d->m_tree, SIGNAL(itemSelectionChanged()), this, SLOT(slot_update_enabled()));

    d->update_enabled();
}

PrvGuiMainControlWidget::~PrvGuiMainControlWidget()
{
    delete d;
}

void PrvGuiMainControlWidget::slot_refresh_tree()
{
    d->m_tree->refresh();
}

void PrvGuiMainControlWidget::slot_update_enabled()
{
    d->update_enabled();
}

void PrvGuiMainControlWidget::slot_upload()
{
    QList<PrvRecord> prvs = d->m_tree->selectedPrvs();
    PrvGuiUploadDialog dlg;
    dlg.setServerNames(d->m_tree->serverNames());
    if (dlg.exec() == QDialog::Accepted) {
        TaskProgress task("Uploading files to server: " + dlg.selectedServer());
        for (int i = 0; i < prvs.count(); i++) {
            task.setProgress((i + 0.3) / prvs.count());
            PrvUpload::initiateUploadToServer(dlg.selectedServer(), prvs[i]);
        }
    }
}

void PrvGuiMainControlWidget::slot_download()
{
    QList<PrvRecord> prvs = d->m_tree->selectedPrvs();
    PrvGuiDownloadDialog dlg;
    dlg.setServerNames(d->m_tree->serverNames());
    if (dlg.exec() == QDialog::Accepted) {
        TaskProgress task("Downloading files to server: " + dlg.selectedServer());
        for (int i = 0; i < prvs.count(); i++) {
            task.setProgress((i + 0.3) / prvs.count());
            PrvDownload::initiateDownloadFromServer(dlg.selectedServer(), prvs[i]);
        }
    }
}

void PrvGuiMainControlWidgetPrivate::update_enabled()
{
    QList<PrvRecord> prvs = m_tree->selectedPrvs();
    find_button("search_again")->setEnabled(true);
    find_button("upload")->setEnabled(!prvs.isEmpty());
    find_button("download")->setEnabled(!prvs.isEmpty());
}

QAbstractButton* PrvGuiMainControlWidgetPrivate::find_button(QString name)
{
    foreach (QAbstractButton* b, m_all_buttons) {
        if (b->property("name").toString() == name)
            return b;
    }
    //let's prevent a crash but give a warning
    qWarning() << "unable to find_button: " + name;
    return (new QPushButton());
}

QPushButton* PrvGuiMainControlWidgetPrivate::create_push_button(QString name, QString label)
{
    QPushButton* button = new QPushButton(label);
    button->setProperty("name", name);
    m_all_buttons << button;
    return button;
}
