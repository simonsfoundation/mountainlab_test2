#ifndef EXPFILTER_H
#define EXPFILTER_H

#include <stdio.h>
#include <QMap>
#include <QVariant>

int extractclips2(
		FILE *infile1,FILE *infile2,
		FILE *infile_TL1,FILE *infile_TL2,
		FILE *outfile1,FILE *outfile2,
		FILE *outfile_TL,FILE *outfile_TM,
		const QMap<QString,QVariant> &params);

#endif // EXPFILTER_H

