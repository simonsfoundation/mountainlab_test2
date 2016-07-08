QT += core gui network

CONFIG += c++11

CONFIG -= app_bundle #Please apple, don't make a bundle

QT += widgets

include(../../mlcommon/mlcommon.pri)
include(../../mlcommon/mlnetwork.pri)

DESTDIR = ../bin
OBJECTS_DIR = ../build
MOC_DIR=../build
TARGET = mountainoverlook
TEMPLATE = app

HEADERS += \
    momainwindow.h \
    mofile.h \
    moresultlistview.h
SOURCES += mountainoverlookmain.cpp \
    momainwindow.cpp \
    mofile.cpp \
    moresultlistview.cpp

