#ifndef DETECT_H
#define DETECT_H

#include <QString>

struct Detect_Opts {
	double detect_threshold;
	int detect_interval;
	int clip_size;
	int sign;
	bool individual_channels;
};

bool detect(const QString &signal_path,const QString &detect_path,const Detect_Opts &opts);


#endif // DETECT_H
