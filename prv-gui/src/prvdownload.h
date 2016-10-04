/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 10/1/2016
*******************************************************/
#ifndef PRVDOWNLOAD_H
#define PRVDOWNLOAD_H

#include "prvupload.h"

namespace PrvDownload {

bool initiateDownloadFromServer(QString server_name, PrvRecord prv);
}

#endif // PRVDOWNLOAD_H
