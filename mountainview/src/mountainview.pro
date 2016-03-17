QT += core gui

CONFIG -= app_bundle #Please apple, don't make a bundle today

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets #We do want to support Qt5, but there is no reason not to use Qt4

DESTDIR = ../bin
OBJECTS_DIR = ../build
MOC_DIR=../build
TARGET = mountainview
TEMPLATE = app

HEADERS += \
    histogramview.h \
    mvstatisticswidget.h \
    mvcrosscorrelogramswidget.h \
    mvlabelcomparewidget.h \
    diskarraymodelclipssubset.h \
    mvcdfview.h \
    static_neuron_colors.h \
    mvutils.h \
    mvoverview2widget.h \
    correlationmatrixview.h \
    mvoverview2widgetcontrolpanel.h \
    mvclusterdetailwidget.h \
    mvclipsview.h \
    mvclusterview.h \
    mvclusterwidget.h \
    mvfiringrateview.h \
    diskarraymodel.h \
    sstimeserieswidget.h \
    sstimeseriesview.h \
    sstimeseriesplot.h \
    ssabstractview.h \
    ssabstractplot.h \
    plotarea.h \
    sslabelsmodel.h \
    sslabelsmodel1.h \
    sscommon.h \
    cvcommon.h
SOURCES += mountainviewmain.cpp \
    histogramview.cpp \
    mvstatisticswidget.cpp \
    mvcrosscorrelogramswidget.cpp \
    mvlabelcomparewidget.cpp \
    diskarraymodelclipssubset.cpp \
    mvcdfview.cpp \
    static_neuron_colors.cpp \
    mvutils.cpp \
    mvoverview2widget.cpp \
    correlationmatrixview.cpp \
    mvoverview2widgetcontrolpanel.cpp \
    mvclusterdetailwidget.cpp \
    mvclipsview.cpp \
    mvclusterview.cpp \
    mvclusterwidget.cpp \
    mvfiringrateview.cpp \
    diskarraymodel.cpp \
    sstimeserieswidget.cpp \
    sstimeseriesview.cpp \
    sstimeseriesplot.cpp \
    ssabstractview.cpp \
    ssabstractplot.cpp \
    plotarea.cpp \
    sslabelsmodel1.cpp \
    sscommon.cpp \
    cvcommon.cpp

INCLUDEPATH += ../../mountainsort/src/utils
DEPENDPATH += ../../mountainsort/src/utils
VPATH += ../../mountainsort/src/utils
HEADERS += get_sort_indices.h msmisc.h
SOURCES += get_sort_indices.cpp msmisc.cpp
HEADERS += get_pca_features.h eigenvalue_decomposition.h
SOURCES += get_pca_features.cpp eigenvalue_decomposition.cpp
HEADERS += get_command_line_params.h
SOURCES += get_command_line_params.cpp
HEADERS += affinetransformation.h
SOURCES += affinetransformation.cpp

INCLUDEPATH += ../../mountainsort/src/mda
DEPENDPATH += ../../mountainsort/src/mda
VPATH += ../../mountainsort/src/mda
HEADERS += mda.h textfile.h diskreadmda.h mdaio.h usagetracking.h
SOURCES += mda.cpp textfile.cpp diskreadmda.cpp mdaio.cpp usagetracking.cpp

INCLUDEPATH += ../../mountainsort/src/processors
DEPENDPATH += ../../mountainsort/src/processors
VPATH += ../../mountainsort/src/processors
HEADERS += extract_clips.h
SOURCES += extract_clips.cpp

RESOURCES += mountainview.qrc

#LAPACK
#On Ubuntu: sudo apt-get install liblapacke-dev
#On CentOS: sudo yum install lapack-devel.i686
INCLUDEPATH += /usr/include/lapacke #this was needed on CentOS
DEFINES += USE_LAPACK
LIBS += -llapack -llapacke
