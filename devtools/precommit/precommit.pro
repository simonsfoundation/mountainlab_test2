QT += core
QT -= gui

TARGET = precommit
CONFIG += console
CONFIG -= app_bundle

DESTDIR = bin
OBJECTS_DIR = build
MOC_DIR = build
TARGET = precommit

TEMPLATE = app

SOURCES += main.cpp

INCLUDEPATH += ../../common/utils
DEPENDPATH += ../../common/utils
VPATH += ../../common/utils
HEADERS += textfile.h
SOURCES += textfile.cpp
