#include "mountainsort_version.h"

#include <QCoreApplication>
#include <QDebug>

#define STRINGIFY(x) #x

QString mountainsort_version()
{
#ifdef MOUNTAINSORT_VERSION
    return STRINGIFY(MOUNTAINSORT_VERSION);
#else
    return "Unknown";
#endif
}
