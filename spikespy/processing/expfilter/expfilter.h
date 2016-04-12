#ifndef EXPFILTER_H
#define EXPFILTER_H

#include <stdio.h>

int expfilter(FILE *infile,FILE *outfile,int lowpass,float tau);

#endif // EXPFILTER_H

