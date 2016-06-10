QT += core gui network

CONFIG += c++11

CONFIG -= app_bundle #Please apple, don't make a bundle

QT += widgets

DESTDIR = ../bin
OBJECTS_DIR = ../build
MOC_DIR=../build
TARGET = mountainoverlook
TEMPLATE = app

HEADERS += \
    momainwindow.h \
    mofile.h \
    mvresultlistview.h \
    moresultlistview.h
SOURCES += mountainoverlookmain.cpp \
    momainwindow.cpp \
    mofile.cpp \
    moresultlistview.cpp

INCLUDEPATH += ../../common/commandlineparams
VPATH += ../../common/commandlineparams
HEADERS += commandlineparams.h
SOURCES += commandlineparams.cpp

INCLUDEPATH += ../../common/utils
VPATH += ../../common/utils
HEADERS += textfile.h
SOURCES += textfile.cpp

INCLUDEPATH += ../../common/cachemanager
DEPENDPATH += ../../common/cachemanager
VPATH += ../../common/cachemanager
HEADERS += cachemanager.h
SOURCES += cachemanager.cpp

INCLUDEPATH += ../../common
DEPENDPATH += ../../common
VPATH += ../../common
HEADERS += mlutils.h
SOURCES += mlutils.cpp
