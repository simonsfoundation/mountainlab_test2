#ifndef MASK_OUT_ARTIFACTS_H
#define MASK_OUT_ARTIFACTS_H

#include <QString>

bool mask_out_artifacts(const QString &signal_path,const QString &signal_out_path,double threshold,int interval_size);

#endif // MASK_OUT_ARTIFACTS_H
