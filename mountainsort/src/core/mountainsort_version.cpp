#include "mountainsort_version.h"
#include "textfile.h"

#include <QCoreApplication>
#include <QDebug>
#include "mlutils.h"

#define STRINGIFY(x) #x

QString mountainsort_version()
{
#ifdef MOUNTAINSORT_VERSION
    return STRINGIFY(MOUNTAINSORT_VERSION);
#else
    return "Unknown";
#endif
}
