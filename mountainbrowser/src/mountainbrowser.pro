QT += core gui
QT += webkitwidgets
QT -= gui
CONFIG -= app_bundle #Please apple, don't make a bundle today :)

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets #We do want to support Qt5, but there is no reason not to use Qt4

DESTDIR = ../bin
OBJECTS_DIR = ../build
MOC_DIR=../build
TARGET = mountainbrowser
TEMPLATE = app

HEADERS += mbmainwindow.h \
    mbexperimentmanager.h \
    mbexperimentlistwidget.h \
    mbexperimentlistwidget2.h
SOURCES += mountainbrowsermain.cpp \
    mbmainwindow.cpp \
    mbexperimentmanager.cpp \
    mbexperimentlistwidget.cpp \
    mbexperimentlistwidget2.cpp

INCLUDEPATH += ../../mountainsort/src/utils
DEPENDPATH += ../../mountainsort/src/utils
VPATH += ../../mountainsort/src/utils
HEADERS += get_command_line_params.h textfile.h
SOURCES += get_command_line_params.cpp textfile.cpp

#QJSON
INCLUDEPATH += ../../mountainsort/src/3rdparty
DEPENDPATH += ../../mountainsort/src/3rdparty
VPATH += ../../mountainsort/src/3rdparty
INCLUDEPATH += ../../mountainsort/src/3rdparty/qjson
DEPENDPATH += ../../mountainsort/src/3rdparty/qjson
VPATH += ../../mountainsort/src/3rdparty/qjson
HEADERS += qjson.h
SOURCES += qjson.cpp
HEADERS += serializer.h serializerrunnable.h parser.h parserrunnable.h json_scanner.h json_parser.hh
SOURCES += serializer.cpp serializerrunnable.cpp parser.cpp parserrunnable.cpp json_scanner.cpp json_parser.cc

DISTFILES += \
    experiments.json \
    html/style.css

RESOURCES += \
    mountainbrowser.qrc


