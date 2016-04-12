#ifndef EXPFILTER_H
#define EXPFILTER_H

#include <stdio.h>
#include <QMap>
#include <QVariant>

int extractclips(FILE *infile,FILE *infile_TL,FILE *outfile,FILE *outfile_TL,FILE *outfile_TM,const QMap<QString,QVariant> &params);

#endif // EXPFILTER_H

