#include "mountainsort_version.h"
#include "textfile.h"

#include <QCoreApplication>
#include <QDebug>

/// Witold, how would you recommend handling versioning? I am terrible at it

QString mountainsort_version()
{
    return read_text_file(mountainlabBasePath() + "/version.txt");
}
