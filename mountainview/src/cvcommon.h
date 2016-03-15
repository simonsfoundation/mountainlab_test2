#ifndef CVCOMMON_H
#define CVCOMMON_H

#include <QDebug>
#include "usagetracking.h"

struct CVPoint {
	float x,y,z;
};
CVPoint cvpoint(float x,float y,float z);

struct CVLine {
	CVPoint p1;
	CVPoint p2;
};
CVLine cvline(float x1,float y1,float z1,float x2,float y2,float z2);
CVLine cvline(CVPoint p1,CVPoint p2);
bool is_zero(CVPoint P);
bool is_zero(CVLine L);
bool compare(CVLine L1,CVLine L2);

void removeOnClose(const QString &path);

struct CVDataPoint {
	CVPoint p;
	int label;
};

QString make_random_id(int numchars);

class CleanupObject : public QObject {
public:
	Q_OBJECT
public slots:
	void closing();
};

#endif // CVCOMMON_H

