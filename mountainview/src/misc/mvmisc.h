#ifndef MVMISC
#define MVMISC

#include "mvcontext.h"

DiskReadMda compute_filtered_firings_remotely(QString mlproxy_url, const DiskReadMda& firings, const MVEventFilter& filter);
DiskReadMda compute_filtered_firings_locally(const DiskReadMda& firings, const MVEventFilter& filter);

#endif // MVMISC
