QT += core gui network

DEFINES += USE_NETWORK
CONFIG += c++11

CONFIG -= app_bundle #Please apple, don't make a bundle today

#we are no longer supporting qt4 for mountainview
#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
QT += widgets

QT+=concurrent

DESTDIR = ../bin
OBJECTS_DIR = ../build
MOC_DIR=../build
TARGET = mountainview
TEMPLATE = app

SOURCES += mountainviewmain.cpp

INCLUDEPATH += core
VPATH += core
HEADERS += \
closemehandler.h flowlayout.h imagesavedialog.h \
mountainprocessrunner.h mvabstractcontextmenuhandler.h \
mvabstractcontrol.h mvabstractview.h mvabstractviewfactory.h \
mvcontrolpanel2.h mvmainwindow.h mvstatusbar.h \
mvcontext.h tabber.h tabberframe.h
SOURCES += \
closemehandler.cpp flowlayout.cpp imagesavedialog.cpp \
mountainprocessrunner.cpp mvabstractcontextmenuhandler.cpp \
mvabstractcontrol.cpp mvabstractview.cpp mvabstractviewfactory.cpp \
mvcontrolpanel2.cpp mvmainwindow.cpp mvstatusbar.cpp \
mvcontext.cpp tabber.cpp tabberframe.cpp
# to remove
HEADERS += computationthread.h
SOURCES += computationthread.cpp

INCLUDEPATH += misc
VPATH += misc
HEADERS += \
clustermerge.h multiscaletimeseries.h \
mvmisc.h mvutils.h
SOURCES += \
clustermerge.cpp multiscaletimeseries.cpp \
mvmisc.cpp mvutils.cpp

INCLUDEPATH += views
VPATH += views
HEADERS += \
correlationmatrixview.h histogramview.h mvamphistview2.h \
mvclipsview.h mvclipswidget.h mvclusterdetailwidget.h \
mvclusterview.h mvclusterwidget.h mvcrosscorrelogramswidget3.h \
mvdiscrimhistview.h mvfiringeventview2.h mvhistogramgrid.h \
mvspikesprayview.h mvtimeseriesrendermanager.h mvtimeseriesview2.h \
mvtimeseriesviewbase.h spikespywidget.h taskprogressview.h
SOURCES += \
correlationmatrixview.cpp histogramview.cpp mvamphistview2.cpp \
mvclipsview.cpp mvclipswidget.cpp mvclusterdetailwidget.cpp \
mvclusterview.cpp mvclusterwidget.cpp mvcrosscorrelogramswidget3.cpp \
mvdiscrimhistview.cpp mvfiringeventview2.cpp mvhistogramgrid.cpp \
mvspikesprayview.cpp mvtimeseriesrendermanager.cpp mvtimeseriesview2.cpp \
mvtimeseriesviewbase.cpp spikespywidget.cpp taskprogressview.cpp

INCLUDEPATH += controlwidgets
VPATH += controlwidgets
HEADERS += \
mvclustervisibilitycontrol.h mveventfiltercontrol.h \
mvexportcontrol.h mvgeneralcontrol.h mvopenviewscontrol.h
SOURCES += \
mvclustervisibilitycontrol.cpp mveventfiltercontrol.cpp \
mvexportcontrol.cpp mvgeneralcontrol.cpp mvopenviewscontrol.cpp

INCLUDEPATH += guides
VPATH += guides
HEADERS += clusterannotationguide.h
SOURCES += clusterannotationguide.cpp

INCLUDEPATH += contextmenuhandlers
VPATH += contextmenuhandlers
HEADERS += mvclustercontextmenuhandler.h
SOURCES += mvclustercontextmenuhandler.cpp

INCLUDEPATH += ../../common/commandlineparams
VPATH += ../../common/commandlineparams
HEADERS += commandlineparams.h
SOURCES += commandlineparams.cpp

INCLUDEPATH += ../../common/utils
VPATH += ../../common/utils
HEADERS += textfile.h haltagent.h taskprogress.h
SOURCES += textfile.cpp haltagent.cpp taskprogress.cpp

INCLUDEPATH += ../../mountainsort/src/utils
DEPENDPATH += ../../mountainsort/src/utils
VPATH += ../../mountainsort/src/utils
HEADERS += get_sort_indices.h msmisc.h
SOURCES += get_sort_indices.cpp msmisc.cpp
HEADERS += get_pca_features.h get_principal_components.h eigenvalue_decomposition.h
SOURCES += get_pca_features.cpp get_principal_components.cpp eigenvalue_decomposition.cpp
HEADERS += affinetransformation.h
SOURCES += affinetransformation.cpp
HEADERS += compute_templates_0.h
SOURCES += compute_templates_0.cpp

DEFINES += USE_REMOTE_MDA
INCLUDEPATH += ../../common/mda
DEPENDPATH += ../../common/mda
VPATH += ../../common/mda
HEADERS += remotereadmda.h diskreadmda.h diskwritemda.h usagetracking.h mda.h mdaio.h
SOURCES += remotereadmda.cpp diskreadmda.cpp diskwritemda.cpp usagetracking.cpp mda.cpp mdaio.cpp

INCLUDEPATH += ../../mountainsort/src/processors
DEPENDPATH += ../../mountainsort/src/processors
VPATH += ../../mountainsort/src/processors
HEADERS += extract_clips.h
SOURCES += extract_clips.cpp

INCLUDEPATH += ../../common/cachemanager
DEPENDPATH += ../../common/cachemanager
VPATH += ../../common/cachemanager
HEADERS += cachemanager.h
SOURCES += cachemanager.cpp

INCLUDEPATH += ../../common
DEPENDPATH += ../../common
VPATH += ../../common
HEADERS += mlutils.h
SOURCES += mlutils.cpp

INCLUDEPATH += ./3rdparty/qaccordion/include
VPATH += ./3rdparty/qaccordion/include
VPATH += ./3rdparty/qaccordion/src
HEADERS += qAccordion/qaccordion.h qAccordion/contentpane.h qAccordion/clickableframe.h
SOURCES += qaccordion.cpp contentpane.cpp clickableframe.cpp

RESOURCES += mountainview.qrc \
    3rdparty/qaccordion/icons/qaccordionicons.qrc

#TODO: Do we need openmp for mountainview?
#OPENMP
!macx {
  QMAKE_LFLAGS += -fopenmp
  QMAKE_CXXFLAGS += -fopenmp
}
#-std=c++11   # AHB removed since not in GNU gcc 4.6.3

