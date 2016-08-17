QT += core gui network

CONFIG += c++11
QMAKE_CXXFLAGS += -Wno-reorder #qaccordion

CONFIG -= app_bundle #Please apple, don't make a bundle today

include(../../mlcommon/mlcommon.pri)
include(../../mlcommon/mda.pri)
include(../../mlcommon/taskprogress.pri)

QT += widgets
QT+=concurrent

DESTDIR = ../bin
OBJECTS_DIR = ../build
MOC_DIR=../build
TARGET = mountainview
TEMPLATE = app

SOURCES += mountaincomparemain.cpp \
    mvmainwindow.cpp

INCLUDEPATH += ../../mountainview/src/core
VPATH += ../../mountainview/src/core
HEADERS += \
closemehandler.h flowlayout.h imagesavedialog.h \
mountainprocessrunner.h mvabstractcontextmenuhandler.h \
mvabstractcontrol.h mvabstractview.h mvabstractviewfactory.h \
mvcontrolpanel2.h mvstatusbar.h \
mvcontext.h tabber.h tabberframe.h taskprogressview.h actionfactory.h \
    mvmainwindow.h

SOURCES += \
closemehandler.cpp flowlayout.cpp imagesavedialog.cpp \
mountainprocessrunner.cpp mvabstractcontextmenuhandler.cpp \
mvabstractcontrol.cpp mvabstractview.cpp mvabstractviewfactory.cpp \
mvcontrolpanel2.cpp mvstatusbar.cpp \
mvcontext.cpp tabber.cpp tabberframe.cpp taskprogressview.cpp actionfactory.cpp

INCLUDEPATH += ../../mountainview/src/misc
VPATH += ../../mountainview/src/misc
HEADERS += \
clustermerge.h multiscaletimeseries.h \
mvmisc.h mvutils.h paintlayer.h paintlayerstack.h
SOURCES += \
clustermerge.cpp multiscaletimeseries.cpp \
mvmisc.cpp mvutils.cpp paintlayer.cpp paintlayerstack.cpp

INCLUDEPATH += ../../mountainview/src/3rdparty/qaccordion/include
VPATH += ../../mountainview/src/3rdparty/qaccordion/include
VPATH += ../../mountainview/src/3rdparty/qaccordion/src
HEADERS += qAccordion/qaccordion.h qAccordion/contentpane.h qAccordion/clickableframe.h
SOURCES += qaccordion.cpp contentpane.cpp clickableframe.cpp
