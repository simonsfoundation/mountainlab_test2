/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 10/1/2016
*******************************************************/
#ifndef PRVUPLOAD_H
#define PRVUPLOAD_H

#include "prvgui.h"

namespace PrvUpload {

bool initiateUploadToServer(QString server_name, PrvRecord prv);
}

void execute_command_in_separate_thread(QString cmd, QStringList args, QObject* on_finished_receiver = 0, const char* sig_or_slot = 0);

#endif // PRVUPLOAD_H
