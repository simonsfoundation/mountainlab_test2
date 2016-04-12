#include "sscommon.h"
#include <QDir>

Vec2 vec2(double x,double y) {
	Vec2 ret; ret.x=x; ret.y=y; return ret;
}

QString ssTempPath() {
	QString ret=QDir::tempPath()+"/spikespy";
	if (!QDir(ret).exists()) {
		QDir(QDir::tempPath()).mkdir("spikespy");
	}
	return ret;
}
