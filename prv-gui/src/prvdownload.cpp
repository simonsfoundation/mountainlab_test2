/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 10/1/2016
*******************************************************/

#include "prvdownload.h"
#include "cachemanager.h"
#include "mlcommon.h"

#include <QJsonDocument>
#include <mlnetwork.h>
#include <taskprogress.h>

bool PrvDownload::initiateDownloadFromServer(QString server_name, PrvRecord prv)
{
    TaskProgress task("Download from server: " + server_name);
    QString local_path = prv.find_local_file();
    if (!local_path.isEmpty()) {
        task.log() << "Already on local disk: " + local_path;
        return false;
    }
    QString remote_url = prv.find_remote_url(server_name);
    if (remote_url.isEmpty()) {
        task.error() << "Unable to find on remote server: " + server_name;
        return false;
    }

    MLNetwork::PrvParallelDownloader* downloader = new MLNetwork::PrvParallelDownloader;
    QObject::connect(downloader, SIGNAL(finished()), downloader, SLOT(deleteLater()));
    downloader->destination_file_name = CacheManager::globalInstance()->makeLocalFile(prv.checksum + ".download_from_server");
    downloader->source_url = remote_url;
    downloader->size = prv.size;
    downloader->num_threads = 5;
    downloader->start();

    return true;
}
