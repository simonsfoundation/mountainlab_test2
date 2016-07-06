QT += core
QT -= gui

CONFIG += c++11

CONFIG -= app_bundle #Please apple, don't make a bundle

DESTDIR = ../bin
OBJECTS_DIR = ../build
MOC_DIR=../build
TARGET = mdaconvert
TEMPLATE = app

HEADERS += \
    mdaconvert.h

SOURCES += mdaconvertmain.cpp \
    mdaconvert.cpp

INCLUDEPATH += ../../mlcommon/include
INCLUDEPATH += ../../mlcommon/include/mda
LIBS += ../../mlcommon/lib/libmlcommon.a
