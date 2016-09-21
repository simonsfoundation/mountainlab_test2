QT += core
QT -= gui
QT += network # for QHostInfo -- not that important
CONFIG -= app_bundle #Please apple, don't make a bundle
CONFIG += c++11

DESTDIR = ../bin
OBJECTS_DIR = ../build
MOC_DIR=../build
TARGET = prv
TEMPLATE = app

SOURCES += prvmain.cpp \
    prvfile.cpp \
    cachemanager.cpp

INCLUDEPATH += util
VPATH += util
HEADERS += sumit.h clparams.h \
    prvfile.h \
    cachemanager.h
SOURCES += sumit.cpp clparams.cpp
