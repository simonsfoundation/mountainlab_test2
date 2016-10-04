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
#include <QJsonDocument>
#include <QMutex>
#include <QProcess>
#include <QSplitter>
#include <QTreeWidget>
#include <taskprogress.h>
#include "taskprogressview.h"
#include "prvguicontrolpanel.h"
#include "mlcommon.h"

class PrvGuiMainWindowPrivate {
public:
    PrvGuiMainWindow* q;

    PrvGuiTreeWidget* m_tree;
    PrvGuiControlPanel* m_control_panel;

    QString m_prv_file_name;
    QJsonObject m_original_object;

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
    d->m_control_panel->addControlWidget("Main", new PrvGuiMainControlWidget(this));
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

    QObject::connect(this, SIGNAL(prvFileNameChanged()), this, SLOT(slot_update_window_title()));
    QObject::connect(d->m_tree, SIGNAL(dirtyChanged()), this, SLOT(slot_update_window_title()));
    slot_update_window_title();
}

PrvGuiMainWindow::~PrvGuiMainWindow()
{
    delete d;
}

void replace_prv(QJsonObject& prv_object, QString original_path, QString new_checksum, long new_size, QString new_checksum1000);
void replace_prv(QJsonArray& prv_array, QString original_path, QString new_checksum, long new_size, QString new_checksum1000)
{
    for (int i = 0; i < prv_array.count(); i++) {
        if (prv_array[i].isArray()) {
            QJsonArray X = prv_array[i].toArray();
            replace_prv(X, original_path, new_checksum, new_size, new_checksum1000);
            prv_array[i] = X;
        }
        else if (prv_array[i].isObject()) {
            QJsonObject X = prv_array[i].toObject();
            replace_prv(X, original_path, new_checksum, new_size, new_checksum1000);
            prv_array[i] = X;
        }
    }
}

void replace_prv(QJsonObject& prv_object, QString original_path, QString new_checksum, long new_size, QString new_checksum1000)
{
    if ((prv_object.contains("original_checksum")) && (prv_object.contains("original_size")) && (prv_object.contains("original_path"))) {
        if (prv_object.value("original_path").toString() == original_path) {
            prv_object["original_checksum"] = new_checksum;
            prv_object["original_size"] = (long long)new_size;
            prv_object["original_checksum_1000"] = new_checksum1000;
        }
    }
    QStringList keys = prv_object.keys();
    foreach (QString key, keys) {
        if (prv_object[key].isArray()) {
            QJsonArray X = prv_object[key].toArray();
            replace_prv(X, original_path, new_checksum, new_size, new_checksum1000);
            prv_object[key] = X;
        }
        else if (prv_object[key].isObject()) {
            QJsonObject X = prv_object[key].toObject();
            replace_prv(X, original_path, new_checksum, new_size, new_checksum1000);
            prv_object[key] = X;
        }
    }
}

bool PrvGuiMainWindow::savePrv(QString prv_file_name)
{
    QList<PrvRecord> prvs = d->m_tree->prvs();
    for (int i = 0; i < prvs.count(); i++) {
        replace_prv(d->m_original_object, prvs[i].original_path, prvs[i].checksum, prvs[i].size, prvs[i].checksum1000);
    }
    QString json = QJsonDocument(d->m_original_object).toJson();
    if (!TextFile::write(prv_file_name, json)) {
        qWarning() << "Unable to write file: " + prv_file_name;
        return false;
    }
    this->setPrvFileName(prv_file_name);
    d->m_tree->setDirty(false);
}

PrvGuiTreeWidget* PrvGuiMainWindow::tree()
{
    return d->m_tree;
}

QString PrvGuiMainWindow::prvFileName() const
{
    return d->m_prv_file_name;
}

void PrvGuiMainWindow::setPrvFileName(QString fname)
{
    if (d->m_prv_file_name == fname)
        return;
    d->m_prv_file_name = fname;
    emit prvFileNameChanged();
}

void PrvGuiMainWindow::slot_update_window_title()
{
    QString title = d->m_prv_file_name;
    if (d->m_tree->isDirty())
        title += " *";
    this->setWindowTitle(title);
}

bool PrvGuiMainWindow::loadPrv(QString prv_file_name)
{
    QJsonParseError err;
    QJsonObject obj = QJsonDocument::fromJson(TextFile::read(prv_file_name).toUtf8(), &err).object();
    if (err.error != QJsonParseError::NoError) {
        printf("Error parsing JSON text.\n");
        return false;
    }
    d->m_original_object = obj;

    QList<PrvRecord> prvs = find_prvs(QFileInfo(prv_file_name).fileName(), obj);
    d->m_tree->setPrvs(prvs);
    this->setPrvFileName(prv_file_name);
    d->m_tree->setDirty(false);

    return true;
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
