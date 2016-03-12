QT += core
QT -= gui
#CONFIG += release
#CONFIG -= debug
CONFIG -= app_bundle #Please apple, don't make a bundle today :)

DESTDIR = ../bin
OBJECTS_DIR = ../build
MOC_DIR=../build
TARGET = mountainsort
TEMPLATE = app


INCLUDEPATH += utils core processors mda

HEADERS += \
    utils/get_command_line_params.h \
    core/msprocessormanager.h \
    core/mountainsort_version.h \
    utils/textfile.h \
    core/msprocessor.h \
    processors/example_processor.h \
    mda/mda.h \
    mda/mdaio.h \
    mda/usagetracking.h \
    mda/diskreadmda.h \
    processors/bandpass_filter_processor.h \
    processors/bandpass_filter0.h \
    core/mstimeserieschunker.h \
    mda/diskwritemda.h

SOURCES += mountainsortmain.cpp \
    utils/get_command_line_params.cpp \
    core/msprocessormanager.cpp \
    core/mountainsort_version.cpp \
    utils/textfile.cpp \
    core/msprocessor.cpp \
    processors/example_processor.cpp \
    mda/mda.cpp \
    mda/mdaio.cpp \
    mda/usagetracking.cpp \
    mda/diskreadmda.cpp \
    processors/bandpass_filter_processor.cpp \
    processors/bandpass_filter0.cpp \
    core/mstimeserieschunker.cpp \
    mda/diskwritemda.cpp

DISTFILES += \
    ../version.txt




