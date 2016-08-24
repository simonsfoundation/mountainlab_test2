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
TARGET = mountaincompare
TEMPLATE = app

INCLUDEPATH += ../../mountainview/src/msv/plugins
VPATH += ../../mountainview/src/msv/plugins
HEADERS += clusterdetailplugin.h
SOURCES += clusterdetailplugin.cpp

INCLUDEPATH += ../../mountainview/src/msv/views
VPATH += ../../mountainview/src/msv/views
HEADERS += clusterdetailview.h
SOURCES += clusterdetailview.cpp

SOURCES += mountaincomparemain.cpp \
    mvmainwindow.cpp \
    mccontext.cpp \
    views/confusionmatrixview.cpp \
    views/matrixview.cpp \
    mcviewfactories.cpp \
    views/compareclusterview.cpp

INCLUDEPATH += ../../mountainview/src/core
VPATH += ../../mountainview/src/core
HEADERS += \
closemehandler.h flowlayout.h imagesavedialog.h \
mountainprocessrunner.h mvabstractcontextmenuhandler.h \
mvabstractcontrol.h mvabstractview.h mvabstractviewfactory.h \
mvcontrolpanel2.h mvstatusbar.h \
mvcontext.h tabber.h tabberframe.h taskprogressview.h actionfactory.h \
    mvmainwindow.h \
    mccontext.h \
    views/confusionmatrixview.h \
    views/matrixview.h \
    mcviewfactories.h \
    views/compareclusterview.h

HEADERS += mvabstractplugin.h
SOURCES += mvabstractplugin.cpp

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

INCLUDEPATH += ../../mountainview/src/controlwidgets
VPATH += ../../mountainview/src/controlwidgets
HEADERS += mvopenviewscontrol.h
SOURCES += mvopenviewscontrol.cpp

INCLUDEPATH += ../../mountainview/src/views
VPATH += ../../mountainview/src/views
HEADERS += \
mvclipsview.h mvclipswidget.h mvtimeseriesview2.h mvtimeseriesviewbase.h mvtimeseriesrendermanager.h \
mvclusterview.h mvclusterwidget.h mvclusterlegend.h
SOURCES += \
mvclipsview.cpp mvclipswidget.cpp mvtimeseriesview2.cpp mvtimeseriesviewbase.cpp mvtimeseriesrendermanager.cpp \
mvclusterview.cpp mvclusterwidget.cpp mvclusterlegend.cpp

INCLUDEPATH += ../../mountainsort/src/processors
DEPENDPATH += ../../mountainsort/src/processors
VPATH += ../../mountainsort/src/processors
HEADERS += extract_clips.h
SOURCES += extract_clips.cpp

#TODO, make a .pri for the accordion because this appears in the mountainview as well
INCLUDEPATH += ../../mountainview/src/3rdparty/qaccordion/include
VPATH += ../../mountainview/src/3rdparty/qaccordion/include
VPATH += ../../mountainview/src/3rdparty/qaccordion/src
HEADERS += qAccordion/qaccordion.h qAccordion/contentpane.h qAccordion/clickableframe.h
SOURCES += qaccordion.cpp contentpane.cpp clickableframe.cpp

INCLUDEPATH += ../../mountainsort/src/utils
DEPENDPATH += ../../mountainsort/src/utils
VPATH += ../../mountainsort/src/utils
HEADERS += get_sort_indices.h msmisc.h
SOURCES += get_sort_indices.cpp msmisc.cpp
HEADERS += affinetransformation.h
SOURCES += affinetransformation.cpp

RESOURCES += ../../mountainview/src/mountainview.qrc \
            ../../mountainview/src/3rdparty/qaccordion/icons/qaccordionicons.qrc
