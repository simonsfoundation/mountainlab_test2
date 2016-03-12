QT += core
QT -= gui
CONFIG += release
CONFIG -= debug
CONFIG -= app_bundle #Please apple, don't make a bundle today :)

DESTDIR = ../bin
OBJECTS_DIR = ../build
MOC_DIR=../build
TARGET = mountainsort
TEMPLATE = app

INCLUDEPATH += utils

SOURCES += mountainsortmain.cpp \
    utils/get_command_line_params.cpp \
    processor_manager.cpp

HEADERS += \
    utils/get_command_line_params.h \
    processor_manager.h


