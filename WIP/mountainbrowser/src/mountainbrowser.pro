QT += core gui
QT += webkitwidgets
QT -= gui
CONFIG -= app_bundle #Please apple, don't make a bundle today :)

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets #We do want to support Qt5, but there is no reason not to use Qt4

DEFINES += USE_NETWORK
CONFIG += c++11

include(../../mlcommon/mlcommon.pri)
include(../../mlcommon/mda.pri)
include(../../mlcommon/taskprogress.pri)
include(../../mlcommon/mlnetwork.pri)

DESTDIR = ../bin
OBJECTS_DIR = ../build
MOC_DIR=../build
TARGET = mountainbrowser
TEMPLATE = app

HEADERS += mountainbrowsermain.h \
    mbcontroller.h
SOURCES += mountainbrowsermain.cpp \
    mbcontroller.cpp

INCLUDEPATH += ../../mountainsort/src/utils
DEPENDPATH += ../../mountainsort/src/utils
VPATH += ../../mountainsort/src/utils
HEADERS +=  msmisc.h
SOURCES += msmisc.cpp

DISTFILES += \
    experiments.json \
    html/style.css \
    franklab.json \
    ../../server/mountainbrowserserver/mountainbrowserserver.py \
    ../../server/mountainbrowserserver/mountainbrowserserver.example.cfg

RESOURCES += \
    mountainbrowser.qrc


