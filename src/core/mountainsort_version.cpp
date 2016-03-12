#include "mountainsort_version.h"
#include "textfile.h"

#include <QCoreApplication>
#include <QDebug>

QString mountainsort_version()
{
	return read_text_file(bin_path()+"/../version.txt");
}

QString bin_path()
{
	return qApp->applicationDirPath();
}
