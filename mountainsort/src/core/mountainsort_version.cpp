#include "mountainsort_version.h"

#include <QCoreApplication>
#include <QDebug>
#include "mlcommon.h"

#define STRINGIFY(x) #x

QString mountainsort_version()
{
#ifdef MOUNTAINSORT_VERSION
    return STRINGIFY(MOUNTAINSORT_VERSION);
#else
    return "Unknown";
#endif
}
