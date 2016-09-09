QT += core
QT -= gui
CONFIG -= app_bundle #Please apple, don't make a bundle today :)

QT += qml

CONFIG += c++11

include(../../mlcommon/mlcommon.pri)
include(../../mlcommon/mda.pri)

DESTDIR = ../bin
OBJECTS_DIR = ../build
MOC_DIR=../build
TARGET = mountainprocess
TEMPLATE = app

INCLUDEPATH += utils core mda unit_tests 3rdparty

HEADERS += \
    mpdaemon.h \
    mpdaemoninterface.h
SOURCES += \
    mpdaemon.cpp \
    mpdaemoninterface.cpp

HEADERS += \
    processmanager.h \
    scriptcontroller.h scriptcontroller2.h \
    unit_tests/unit_tests.h

SOURCES += \
    processmanager.cpp \
    scriptcontroller.cpp scriptcontroller2.cpp \
    unit_tests/unit_tests.cpp

#tests
test {
    QT += testlib
    CONFIG += testcase
    TARGET = mountainsort_test
    DEPENDPATH += unit_tests
    SOURCES += unit_tests/testMda.cpp	\
	unit_tests/testMain.cpp	\
	unit_tests/testMdaIO.cpp
    HEADERS += unit_tests/testMda.h \
	unit_tests/testMdaIO.h
} else {
    SOURCES += mountainprocessmain.cpp
}
