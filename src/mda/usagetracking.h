#ifndef USAGETRACKING
#define USAGETRACKING

#include <stdlib.h>
#include <stdio.h>

FILE *jfopen(const char *path,const char *mode);
void jfclose(FILE *F);
int jfread(void *data,size_t sz,int num,FILE *F);
int jfwrite(void *data,size_t sz,int num,FILE *F);
int jnumfilesopen();

void *jmalloc(size_t num_bytes);
void jfree(void *ptr);
int jmalloccount();
int64_t jbytesallocated();
int jnumbytesread();
int jnumbyteswritten();

#endif // USAGETRACKING

