#ifndef REMOVE_DUPLICATES_H
#define REMOVE_DUPLICATES_H

#include <QString>

bool remove_duplicates_v1(const QString &firings_in_path,const QString &firings_out_path,int max_dt=6,float overlap_threshold=0.25);

#endif // REMOVE_DUPLICATES_H

