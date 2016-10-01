QT += core
QT -= gui
QT += network
CONFIG -= app_bundle #Please apple, don't make a bundle
CONFIG += c++11

# for gui mode only
QT += gui widgets
INCLUDEPATH += ../../mountainview/src/core
HEADERS += ../../mountainview/src/core/taskprogressview.h
SOURCES += ../../mountainview/src/core/taskprogressview.cpp

DESTDIR = ../bin
OBJECTS_DIR = ../build
MOC_DIR=../build
TARGET = prv
TEMPLATE = app

SOURCES += prvmain.cpp \
    prvfile.cpp

include(../../mlcommon/mlcommon.pri)
include(../../mlcommon/mlnetwork.pri)
include(../../mlcommon/taskprogress.pri)
