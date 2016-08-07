#-------------------------------------------------
#
# Project created by QtCreator 2016-08-07T21:59:15
#
#-------------------------------------------------

QT       += testlib

QT       -= gui

TARGET = tst_mdareadertest
CONFIG   += console c++11
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += tst_mdareadertest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
include(../../../mlcommon/mlcommon.pri)
