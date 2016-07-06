QT += core gui
QT += widgets

CONFIG += c++11
CONFIG -= app_bundle
CONFIG += staticlib

DESTDIR = ../lib
OBJECTS_DIR = ../build
MOC_DIR=../build
TARGET = mlcommon
TEMPLATE = lib

INCLUDEPATH += ../include
VPATH += ../include
HEADERS += mlcommon.h

SOURCES += \
    mlcommon.cpp

INCLUDEPATH += ../include/mda
VPATH += ../include/mda
VPATH += mda
HEADERS += diskreadmda.h diskwritemda.h mda.h mdaio.h remotereadmda.h usagetracking.h
SOURCES += diskreadmda.cpp diskwritemda.cpp mda.cpp mdaio.cpp remotereadmda.cpp usagetracking.cpp

INCLUDEPATH += ../include/cachemanager
VPATH += ../include/cachemanager
VPATH += cachemanager
HEADERS += cachemanager.h
SOURCES += cachemanager.cpp

INCLUDEPATH += ../include/taskprogress
VPATH += ../include/taskprogress
VPATH += taskprogress
HEADERS += taskprogress.h
SOURCES += taskprogress.cpp

INCLUDEPATH += ../include/mlnetwork
VPATH += ../include/mlnetwork
VPATH += mlnetwork
HEADERS += mlnetwork.h
SOURCES += mlnetwork.cpp
