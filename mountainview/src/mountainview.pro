QT += core gui script network

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

HEADERS += \
    histogramview.h \
    mvutils.h \
    mvmainwindow.h \
    correlationmatrixview.h \
    mvclusterdetailwidget.h \
    mvclipsview.h \
    mvclusterview.h \
    mvclusterwidget.h \
    mvfiringeventview.h \
    mvfiringeventview2.h \
    tabber.h \
    imagesavedialog.h \
    closemehandler.h \
    computationthread.h \
    mvcrosscorrelogramswidget2.h \
    mountainprocessrunner.h \
    mvclipswidget.h \
    taskprogressview.h \
    mvcontrolpanel.h \
    flowlayout.h \
    clustermerge.h \
    mvviewagent.h \
    mvstatusbar.h \
    mvtimeseriesview.h \
    mvtimeseriesview2.h \
    multiscaletimeseries.h \
    mvtimeseriesrendermanager.h \
    spikespywidget.h \
    mvspikesprayview.h \
    mvfile.h \
    mvtimeseriesviewbase.h \
    mvabstractview.h \
    mvmisc.h \
    mvamphistview.h \
    mvdiscrimhistview.h \
    tabberframe.h \
    mvclustercontextmenu.h \
    mvhistogramgrid.h \
    mvcrosscorrelogramswidget3.h \
    mvamphistview2.h \
    mvabstractviewfactory.h \
    clusterannotationguide.h \
    mvabstractcontextmenuhandler.h \
    mvclustercontextmenuhandler.h \
    mvabstractcontrol.h \
    mvcontrolpanel2.h \
    mvgeneralcontrol.h \
    mveventfiltercontrol.h \
    mvopenviewscontrol.h \
    mvclustervisibilitycontrol.h \
    mvexportcontrol.h \


SOURCES += mountainviewmain.cpp \
    histogramview.cpp \
    mvutils.cpp \
    mvmainwindow.cpp \
    correlationmatrixview.cpp \
    mvclusterdetailwidget.cpp \
    mvclipsview.cpp \
    mvclusterview.cpp \
    mvclusterwidget.cpp \
    mvfiringeventview.cpp \
    mvfiringeventview2.cpp \
    tabber.cpp \
    imagesavedialog.cpp \
    closemehandler.cpp \
    computationthread.cpp \
    mvcrosscorrelogramswidget2.cpp \
    mountainprocessrunner.cpp \
    mvclipswidget.cpp \
    taskprogressview.cpp \
    mvcontrolpanel.cpp \
    flowlayout.cpp \
    clustermerge.cpp \
    mvviewagent.cpp \
    mvstatusbar.cpp \
    mvtimeseriesview.cpp \
    mvtimeseriesview2.cpp \
    multiscaletimeseries.cpp \
    mvtimeseriesrendermanager.cpp \
    spikespywidget.cpp \
    mvspikesprayview.cpp \
    mvfile.cpp \
    mvtimeseriesviewbase.cpp \
    mvabstractview.cpp \
    mvmisc.cpp \
    mvamphistview.cpp \
    mvdiscrimhistview.cpp \
    tabberframe.cpp \
    mvclustercontextmenu.cpp \
    mvhistogramgrid.cpp \
    mvcrosscorrelogramswidget3.cpp \
    mvamphistview2.cpp \
    mvabstractviewfactory.cpp \
    clusterannotationguide.cpp \
    mvabstractcontextmenuhandler.cpp \
    mvclustercontextmenuhandler.cpp \
    mvabstractcontrol.cpp \
    mvcontrolpanel2.cpp \
    mvgeneralcontrol.cpp \
    mveventfiltercontrol.cpp \
    mvopenviewscontrol.cpp \
    mvclustervisibilitycontrol.cpp \
    mvexportcontrol.cpp

#HEADERS += sstimeserieswidget.h \
#    sstimeseriesview.h \
#    sstimeseriesplot.h \
#    ssabstractview.h \
#    ssabstractplot.h \
#    sslabelsmodel.h \
#    sslabelsmodel1.h \
#    sscommon.h plotarea.h

#SOURCES += sstimeserieswidget.cpp \
#    sstimeseriesview.cpp \
#    sstimeseriesplot.cpp \
#    ssabstractview.cpp \
#    ssabstractplot.cpp \
#    sslabelsmodel1.cpp \
#    sscommon.cpp plotarea.cpp

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

#LAPACK
#On Ubuntu: sudo apt-get install liblapacke-dev
#On CentOS: sudo yum install lapack-devel.i686
#INCLUDEPATH += /usr/include/lapacke #this was needed on CentOS
#DEFINES += USE_LAPACK
#LIBS += -llapack -llapacke

#OPENMP
!macx {
  QMAKE_LFLAGS += -fopenmp
  QMAKE_CXXFLAGS += -fopenmp
}
#-std=c++11   # AHB removed since not in GNU gcc 4.6.3

DISTFILES += \
    ../../server/mountainviewserver/mountainviewserver.py \
    ../../server/mscmdserver/mscmdserver.py \
    ../../server/mdaserver/mdaserver.py \
    ../../server/mountainviewserver/mountainviewserver.example.cfg \
    ../../server/mdaserver/mdaserver.example.cfg \
    ../../server/mscmdserver/mscmdserver.example.cfg
