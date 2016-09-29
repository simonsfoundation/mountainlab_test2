QT += core
QT -= gui
QT += network
CONFIG -= app_bundle #Please apple, don't make a bundle
CONFIG += c++11

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
