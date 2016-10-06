/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 10/1/2016
*******************************************************/

#include "prvguidownloaddialog.h"
#include "prvguimaincontrolwidget.h"
#include "prvguitreewidget.h"
#include "prvguiuploaddialog.h"
#include <QGridLayout>
#include <QPushButton>
#include <QDebug>
#include <taskprogress.h>
#include <mountainprocessrunner.h>
#include "prvupload.h"
#include "prvdownload.h"
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include "mlcommon.h"
#include <QCoreApplication>
#include "mlnetwork.h"

class PrvGuiMainControlWidgetPrivate {
public:
    PrvGuiMainControlWidget* q;
    PrvGuiMainWindow* m_main_window;
    PrvGuiTreeWidget* m_tree;
    QList<QAbstractButton*> m_all_buttons;

    void update_enabled();
    QAbstractButton* find_button(QString name);
    QPushButton* create_push_button(QString name, QString label);
    bool regenerate_prv(const PrvRecord& prv);
};

PrvGuiMainControlWidget::PrvGuiMainControlWidget(PrvGuiMainWindow* MW)
{
    d = new PrvGuiMainControlWidgetPrivate;
    d->q = this;

    d->m_main_window = MW;
    d->m_tree = MW->tree();

    QGridLayout* grid_layout = new QGridLayout;
    this->setLayout(grid_layout);
    int row = 0;

    {
        QPushButton* button = d->create_push_button("search again", "Search again");
        QObject::connect(button, SIGNAL(clicked(bool)), this, SLOT(slot_search_again()));
        grid_layout->addWidget(button, row, 0, 1, 2);
    }
    row++;

    {
        QPushButton* button = d->create_push_button("upload", "Upload...");
        button->setToolTip("Upload selected items to a server");
        QObject::connect(button, SIGNAL(clicked(bool)), this, SLOT(slot_upload()));
        grid_layout->addWidget(button, row, 0, 1, 1);
    }
    {
        QPushButton* button = d->create_push_button("download", "Download...");
        button->setToolTip("Download selected items from a server");
        QObject::connect(button, SIGNAL(clicked(bool)), this, SLOT(slot_download()));
        grid_layout->addWidget(button, row, 1, 1, 1);
    }
    row++;

    {
        QPushButton* button = d->create_push_button("regenerate", "Regenerate...");
        button->setToolTip("Regenerate selected items using processing");
        QObject::connect(button, SIGNAL(clicked(bool)), this, SLOT(slot_regenerate()));
        grid_layout->addWidget(button, row, 0, 1, 1);
    }
    row++;

    {
        QPushButton* button = d->create_push_button("save", "Save");
        button->setToolTip("Save the .prv file. Modifications may have occurred when files were regenerated.");
        QObject::connect(button, SIGNAL(clicked(bool)), this, SLOT(slot_save()));
        grid_layout->addWidget(button, row, 0, 1, 1);
    }
    {
        QPushButton* button = d->create_push_button("saveas", "Save as...");
        button->setToolTip("Save the .prv file. Modifications may have occurred when files were regenerated.");
        QObject::connect(button, SIGNAL(clicked(bool)), this, SLOT(slot_save_as()));
        grid_layout->addWidget(button, row, 1, 1, 1);
    }
    row++;

    QObject::connect(d->m_tree, SIGNAL(itemSelectionChanged()), this, SLOT(slot_update_enabled()));

    d->update_enabled();
}

PrvGuiMainControlWidget::~PrvGuiMainControlWidget()
{
    delete d;
}

void PrvGuiMainControlWidget::slot_search_again()
{
    d->m_tree->startAllSearches();
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
            MLNetwork::PrvParallelUploader* uploader = PrvUpload::initiateUploadToServer(dlg.selectedServer(), prvs[i]);
            if (uploader) {
                uploader->setProperty("checksum", prvs[i].checksum);
                uploader->setProperty("size", (long long)prvs[i].size);
                uploader->setProperty("server", dlg.selectedServer());
                QObject::connect(uploader, SIGNAL(finished()), this, SLOT(slot_uploader_finished));
            }
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

void PrvGuiMainControlWidget::slot_regenerate()
{
    QList<PrvRecord> prvs = d->m_tree->selectedPrvs();
    TaskProgress task(QString("Regenerating %1 files").arg(prvs.count()));
    for (int i = 0; i < prvs.count(); i++) {
        task.setProgress((i + 0.5) / prvs.count());
        if (!d->regenerate_prv(prvs[i])) {
            task.error() << "Error regenerating prv file.";
        }
    }
}

void PrvGuiMainControlWidget::slot_save()
{
    d->m_main_window->savePrv(d->m_main_window->prvFileName());
}

void PrvGuiMainControlWidget::slot_save_as()
{
    QString fname = QFileDialog::getSaveFileName(this, "Save PRV file as", d->m_main_window->prvFileName(), "*.prv");
    if (fname.isEmpty())
        return;
    d->m_main_window->savePrv(fname);
}

void PrvGuiMainControlWidget::slot_uploader_finished()
{
    MLNetwork::PrvParallelUploader* uploader = qobject_cast<MLNetwork::PrvParallelUploader*>(sender());
    if (!uploader)
        return;
    QString checksum = uploader->property("checksum").toString();
    long size = uploader->property("checksum").toLongLong();
    QString server = uploader->property("server").toString();
    d->m_main_window->searchAgain(checksum, size, server);
}

void PrvGuiMainControlWidgetPrivate::update_enabled()
{
    QList<PrvRecord> prvs = m_tree->selectedPrvs();
    find_button("search_again")->setEnabled(true);
    find_button("upload")->setEnabled(!prvs.isEmpty());
    find_button("download")->setEnabled(!prvs.isEmpty());
    find_button("save")->setEnabled(m_tree->isDirty());
    find_button("saveas")->setEnabled(true);
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

class run_process_thread : public QThread {
public:
    MountainProcessRunner MPR;
    void run()
    {
        MPR.runProcess();
    }
};

QString run_process(PrvProcessRecord& P, QString& output_pname)
{
    run_process_thread T;
    T.MPR.setProcessorName(P.processor_name);

    QVariantMap params = P.parameters;

    QStringList ikeys = P.inputs.keys();
    foreach (QString ikey, ikeys) {
        QString input_file = P.inputs[ikey].find_local_file();
        if (input_file.isEmpty()) {
            qWarning() << "Unexpected error. Unable to find input file on local disk, even though we made it into run_process.";
            return "";
        }
        params[ikey] = input_file;
    }

    T.MPR.setInputParameters(params);

    QVariantMap outputs0;
    QStringList okeys = P.outputs.keys();
    foreach (QString okey, okeys) {
        outputs0[okey] = T.MPR.makeOutputFilePath(okey);
    }

    T.start();
    while (!T.isFinished()) {
        T.wait(10);
        qApp->processEvents();
    }

    QString fname = outputs0[output_pname].toString();
    if (!QFile::exists(fname)) {
        qWarning() << "+++++++++++++++++ Problem in run_process. File does not exist: " + fname;
        qDebug() << outputs0 << output_pname;
        return "";
    }

    return fname;
}

bool PrvGuiMainControlWidgetPrivate::regenerate_prv(const PrvRecord& prv)
{
    TaskProgress task("Regenerating " + prv.label);
    QString path0 = prv.find_local_file();
    if (!path0.isEmpty()) {
        task.log() << "File already found on disk: " + path0;
        return true;
    }
    QString output_pname;
    int process_index = find_process_corresponding_to_output(prv.processes, prv.original_path, output_pname);
    if (process_index < 0) {
        task.error() << "Unable to find process with output corresponding to prv: " + prv.label + " " + prv.original_path;
        return false;
    }
    PrvProcessRecord P = prv.processes[process_index];
    if (P.processor_name.isEmpty()) {
        task.error() << "Unable to find process corresponding to object: " + prv.original_path;
        return false;
    }
    QStringList ikeys = P.inputs.keys();
    foreach (QString ikey, ikeys) {
        PrvRecord prv0 = P.inputs[ikey];
        prv0.processes.append(prv.processes);
        task.log() << "Input " + ikey;
        if (!regenerate_prv(prv0)) {
            task.error() << "Problem regenerating input: " + ikey + ": " + prv0.original_path;
            return false;
        }
        P.inputs[ikey] = prv0; //because the checksum of this or a process may have changed!
    }

    QString output_file_path = run_process(P, output_pname);
    if (output_file_path.isEmpty()) {
        task.error() << "Problem running process ****: " + P.processor_name;
        return false;
    }

    //The checksum and/or size may have changed
    PrvRecord prv_new = prv;
    prv_new.checksum = MLUtil::computeSha1SumOfFile(output_file_path);
    prv_new.size = QFileInfo(output_file_path).size();
    prv_new.checksum1000 = MLUtil::computeSha1SumOfFileHead(output_file_path, 1000);
    if (prv.checksum != prv_new.checksum) {
        QString str = QString("Note: the checksum or the prv object has changed %1 <> %2, sizes: %3, %4").arg(prv_new.checksum).arg(prv.checksum).arg(prv_new.size).arg(prv.size);
        task.log() << str;
        printf("%s\n", str.toUtf8().data());
    }
    m_tree->replacePrv(prv.original_path, prv_new);
    m_tree->refresh();

    return true;
}
