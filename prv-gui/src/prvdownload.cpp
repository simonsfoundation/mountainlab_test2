/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 10/1/2016
*******************************************************/

#include "prvdownload.h"
#include "cachemanager.h"
#include "mlcommon.h"

#include <QJsonDocument>

void PrvDownload::initiateDownloadFromServer(QString server_name, PrvRecord prv)
{
    QString tmp_fname = CacheManager::globalInstance()->makeLocalFile(MLUtil::makeRandomId(10) + ".PrvManagerdlg.prv");
    TextFile::write(tmp_fname, QJsonDocument(prv.original_object).toJson());
    QString cmd = "prv";
    QStringList args;
    args << "ensure-local" << tmp_fname << "--server=" + server_name;
    execute_command_in_separate_thread(cmd, args);
}
