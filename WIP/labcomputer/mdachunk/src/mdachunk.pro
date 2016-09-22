#-------------------------------------------------
#
# Project created by QtCreator 2016-03-30T06:59:21
#
#-------------------------------------------------

QT       += core network

QT       -= gui

CONFIG += c++11

include(../../../mlcommon/mlcommon.pri)
include(../../../mlcommon/mda.pri)

DESTDIR = ../bin
OBJECTS_DIR = ../build
MOC_DIR=../build
TARGET = mdachunk
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp

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


