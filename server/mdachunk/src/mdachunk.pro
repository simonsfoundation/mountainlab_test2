#-------------------------------------------------
#
# Project created by QtCreator 2016-03-30T06:59:21
#
#-------------------------------------------------

QT       += core network

QT       -= gui

DESTDIR = ../bin
OBJECTS_DIR = ../build
MOC_DIR=../build
TARGET = mdachunk
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

INCLUDEPATH += ../../../mountainsort/src/mda
DEPENDPATH += ../../../mountainsort/src/mda
VPATH += ../../../mountainsort/src/mda
SOURCES += main.cpp \
    diskreadmda.cpp \
    diskwritemda.cpp \
    mda.cpp \
    mdaio.cpp \
    remotereadmda.cpp \
    usagetracking.cpp \
    utils/get_command_line_params.cpp \
    utils/textfile.cpp
INCLUDEPATH += utils
HEADERS += \
    diskreadmda.h \
    diskwritemda.h \
    mda.h \
    mdaio.h \
    usagetracking.h \
    remotereadmda.h \
    utils/get_command_line_params.h \
    utils/textfile.h

INCLUDEPATH += ../../../mountainsort/src/utils
DEPENDPATH += ../../../mountainsort/src/utils
VPATH += ../../../mountainsort/src/utils
HEADERS += msmisc.h
SOURCES += msmisc.cpp

INCLUDEPATH += ../../../mountainsort/src/core
DEPENDPATH += ../../../mountainsort/src/core
VPATH += ../../../mountainsort/src/core
HEADERS += mscachemanager.h
SOURCES += mscachemanager.cpp


