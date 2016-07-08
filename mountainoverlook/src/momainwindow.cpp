#include "momainwindow.h"
#include "moresultlistview.h"

#include <QFile>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QMessageBox>
#include <QProcess>
#include <cachemanager.h>

#include "mlcommon.h"

class MOMainWindowPrivate {
public:
    MOMainWindow* q;

    MOResultListView* m_result_list_view;
    MOFile m_mof;
};

MOMainWindow::MOMainWindow()
{
    d = new MOMainWindowPrivate;
    d->q = this;
    d->m_result_list_view = new MOResultListView(&d->m_mof);

    QHBoxLayout* hlayout = new QHBoxLayout;
    hlayout->addWidget(d->m_result_list_view);
    this->setLayout(hlayout);

    QObject::connect(d->m_result_list_view, SIGNAL(resultActivated(QString)), this, SLOT(slot_open_result(QString)));
}

MOMainWindow::~MOMainWindow()
{
    delete d;
}

void MOMainWindow::read(const QString& mof_path)
{
    d->m_mof.read(mof_path);
}

void MOMainWindow::load(const QString& mof_url)
{
    d->m_mof.load(mof_url);
}

void MOMainWindow::slot_open_result(QString name)
{
    if (name.isEmpty())
        return;
    QJsonObject obj = d->m_mof.result(name);
    QString mv_json = QJsonDocument(obj["mv"].toObject()).toJson();
    QString path = CacheManager::globalInstance()->makeLocalFile(name + ".mv", CacheManager::LongTerm);
    if (!TextFile::write(path, mv_json)) {
        QMessageBox::critical(0, "Unable to write file", "Unable to write: " + path);
        return;
    }
    QString mv_exe = MLUtil::mountainlabBasePath() + "/mountainview/bin/mountainview";
    if (!QFile::exists(mv_exe)) {
        QMessageBox::critical(0, "Unable to open mountainview", "MountainView executable does not exist: " + mv_exe);
        return;
    }
    QStringList args;
    args << path;
    if (!QProcess::startDetached(mv_exe, args)) {
        QMessageBox::critical(0, "Problem running MountainView", "Error running: " + mv_exe);
    }
}
