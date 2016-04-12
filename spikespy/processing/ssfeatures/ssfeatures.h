#ifndef EXPFILTER_H
#define EXPFILTER_H

#include <stdio.h>
#include <QMap>
#include <QVariant>

int ssfeatures(FILE *infile,FILE *outfile,const QMap<QString,QVariant> &params);

#endif // EXPFILTER_H

