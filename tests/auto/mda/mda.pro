#-------------------------------------------------
#
# Project created by QtCreator 2016-08-06T21:48:13
#
#-------------------------------------------------

QT       += testlib

QT       -= gui

TARGET = tst_mdatest
CONFIG   += console c++11
CONFIG   -= app_bundle

TEMPLATE = app

include(../../../mlcommon/mlcommon.pri)

SOURCES += tst_mdatest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
