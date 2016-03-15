#ifndef SSCOMMON_H
#define SSCOMMON_H

#include <QString>
#include <QApplication>

//multiscale factor of 2 uses the most extra disk space, but provides the smoothest browsing experience
//This is because depending on the zoom factor, more points need to be rendered/copying with 3+
// (certain zoom factors are fine, but other unlucky zoom factors cause a bit of a delay in rendering/copying)
//This becomes expecially noticable for multiscale factors>=10
#define MULTISCALE_FACTOR 2

#include "diskarraymodel.h"
#include "usagetracking.h"
#define SSARRAY DiskArrayModel

struct Vec2 {
	double x,y;
};
Vec2 vec2(double x,double y);

QString ssTempPath();

#include <QDebug>  // ahb

#endif // SSCOMMON_H

