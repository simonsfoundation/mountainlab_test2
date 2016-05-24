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

INCLUDEPATH += ../../common
DEPENDPATH += ../../common
VPATH += ../../common
HEADERS += textfile.h
SOURCES += textfile.cpp
