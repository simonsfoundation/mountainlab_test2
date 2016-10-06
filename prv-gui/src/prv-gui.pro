QT += core gui network widgets
CONFIG -= app_bundle #Please apple, don't make a bundle
CONFIG += c++11

DESTDIR = ../bin
OBJECTS_DIR = ../build
MOC_DIR=../build
TARGET = prv-gui
TEMPLATE = app

SOURCES += prv-guimain.cpp \
    prvguimainwindow.cpp \
    ../../mountainview/src/core/taskprogressview.cpp \
    prvguicontrolpanel.cpp \
    prvguitreewidget.cpp \
    prvgui.cpp \
    prvguimaincontrolwidget.cpp \
    prvguiuploaddialog.cpp \
    prvupload.cpp \
    prvdownload.cpp \
    prvguidownloaddialog.cpp \
    prvguiitemdetailwidget.cpp \
    locatemanager.cpp \
    locatemanagerworker.cpp

include(../../mlcommon/mlcommon.pri)
include(../../mlcommon/mlnetwork.pri)
include(../../mlcommon/taskprogress.pri)

INCLUDEPATH += ../../mountainview/src/core
HEADERS += \
    prvguimainwindow.h \
    ../../mountainview/src/core/taskprogressview.h \
    prvguicontrolpanel.h \
    prvguitreewidget.h \
    prvgui.h \
    prvguimaincontrolwidget.h \
    prvguiuploaddialog.h \
    prvupload.h \
    prvdownload.h \
    prvguidownloaddialog.h \
    prvguiitemdetailwidget.h \
    locatemanager.h \
    locatemanagerworker.h

INCLUDEPATH += ../../mountainview/src/core
HEADERS += ../../mountainview/src/core/mountainprocessrunner.h
SOURCES += ../../mountainview/src/core/mountainprocessrunner.cpp

INCLUDEPATH += ../../mountainview/src/3rdparty/qaccordion/include
VPATH += ../../mountainview/src/3rdparty/qaccordion
HEADERS += include/qAccordion/qaccordion.h include/qAccordion/contentpane.h include/qAccordion/clickableframe.h
SOURCES += src/qaccordion.cpp src/contentpane.cpp src/clickableframe.cpp

RESOURCES += icons/qaccordionicons.qrc

FORMS += \
    prvguiuploaddialog.ui prvguidownloaddialog.ui
