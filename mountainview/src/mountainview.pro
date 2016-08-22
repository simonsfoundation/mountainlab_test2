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

SOURCES += mountainviewmain.cpp \
    guides/individualmergedecisionpage.cpp \
    views/mvspikespraypanel.cpp \
    views/firetrackview.cpp \
    views/ftelectrodearrayview.cpp \
    controlwidgets/mvmergecontrol.cpp \
    controlwidgets/mvprefscontrol.cpp \
    views/mvpanelwidget.cpp views/mvpanelwidget2.cpp \
    views/mvtemplatesview2.cpp \
    views/mvtemplatesview2panel.cpp \
    core/mvplugin.cpp

INCLUDEPATH += core
VPATH += core
HEADERS += \
closemehandler.h flowlayout.h imagesavedialog.h \
mountainprocessrunner.h mvabstractcontextmenuhandler.h \
mvabstractcontrol.h mvabstractview.h mvabstractviewfactory.h \
mvcontrolpanel2.h mvmainwindow.h mvstatusbar.h \
mvcontext.h tabber.h tabberframe.h taskprogressview.h actionfactory.h \
    guides/individualmergedecisionpage.h \
    views/mvspikespraypanel.h \
    views/firetrackview.h \
    views/ftelectrodearrayview.h \
    controlwidgets/mvmergecontrol.h \
    controlwidgets/mvprefscontrol.h \
    views/mvpanelwidget.h views/mvpanelwidget2.h \
    views/mvtemplatesview2.h \
    views/mvtemplatesview2panel.h \
    core/mvplugin.h

SOURCES += \
closemehandler.cpp flowlayout.cpp imagesavedialog.cpp \
mountainprocessrunner.cpp mvabstractcontextmenuhandler.cpp \
mvabstractcontrol.cpp mvabstractview.cpp mvabstractviewfactory.cpp \
mvcontrolpanel2.cpp mvmainwindow.cpp mvstatusbar.cpp \
mvcontext.cpp tabber.cpp tabberframe.cpp taskprogressview.cpp actionfactory.cpp
# to remove
HEADERS += computationthread.h
SOURCES += computationthread.cpp

HEADERS += guides/guidev1.h guides/guidev2.h
SOURCES += guides/guidev1.cpp guides/guidev2.cpp

INCLUDEPATH += misc
VPATH += misc
HEADERS += \
clustermerge.h multiscaletimeseries.h \
mvmisc.h mvutils.h paintlayer.h paintlayerstack.h
SOURCES += \
clustermerge.cpp multiscaletimeseries.cpp \
mvmisc.cpp mvutils.cpp paintlayer.cpp paintlayerstack.cpp

INCLUDEPATH += views
VPATH += views
HEADERS += \
correlationmatrixview.h histogramview.h mvamphistview2.h mvamphistview3.h histogramlayer.h \
mvclipsview.h mvclipswidget.h mvclusterdetailwidget.h \
mvclusterview.h mvclusterwidget.h mvcrosscorrelogramswidget3.h \
mvdiscrimhistview.h mvfiringeventview2.h mvhistogramgrid.h \
mvspikesprayview.h mvtimeseriesrendermanager.h mvtimeseriesview2.h \
mvtimeseriesviewbase.h spikespywidget.h mvdiscrimhistview_guide.h \
mvclusterlegend.h
SOURCES += \
correlationmatrixview.cpp histogramview.cpp mvamphistview2.cpp mvamphistview3.cpp histogramlayer.cpp \
mvclipsview.cpp mvclipswidget.cpp mvclusterdetailwidget.cpp \
mvclusterview.cpp mvclusterwidget.cpp mvcrosscorrelogramswidget3.cpp \
mvdiscrimhistview.cpp mvfiringeventview2.cpp mvhistogramgrid.cpp \
mvspikesprayview.cpp mvtimeseriesrendermanager.cpp mvtimeseriesview2.cpp \
mvtimeseriesviewbase.cpp spikespywidget.cpp mvdiscrimhistview_guide.cpp \
mvclusterlegend.cpp

INCLUDEPATH += controlwidgets
VPATH += controlwidgets
HEADERS += \
mvclustervisibilitycontrol.h \
mvexportcontrol.h mvgeneralcontrol.h mvopenviewscontrol.h
SOURCES += \
mvclustervisibilitycontrol.cpp \
mvexportcontrol.cpp mvgeneralcontrol.cpp mvopenviewscontrol.cpp

INCLUDEPATH += guides
VPATH += guides
HEADERS += clusterannotationguide.h
SOURCES += clusterannotationguide.cpp

INCLUDEPATH += contextmenuhandlers
VPATH += contextmenuhandlers
HEADERS += mvclustercontextmenuhandler.h mvclusterpaircontextmenuhandler.h
SOURCES += mvclustercontextmenuhandler.cpp mvclusterpaircontextmenuhandler.cpp

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

INCLUDEPATH += ../../mountainsort/src/processors
DEPENDPATH += ../../mountainsort/src/processors
VPATH += ../../mountainsort/src/processors
HEADERS += extract_clips.h
SOURCES += extract_clips.cpp

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

