TEMPLATE = lib
CONFIG += staticlib
TARGET = mountainsortlib

QT = core

CONFIG -= app_bundle #Please apple, don't make a bundle today :)

DESTDIR = ../build
OBJECTS_DIR = ../build
MOC_DIR=../build


INCLUDEPATH += utils core processors mda unit_tests 3rdparty isosplit ../../common/cachemanager

SOURCES += \ #see below
    isosplit/isosplit2.cpp \
    isosplit/isocut.cpp \
    isosplit/jisotonic.cpp

SOURCES += utils/eigenvalue_decomposition.cpp #see below
DEFINES += USE_REMOTE_MDA
DEFINES += USE_SSE2
INCLUDEPATH += ../../common/mda
DEPENDPATH += ../../common/mda
VPATH += ../../common/mda
INCLUDEPATH += ../../common
DEPENDPATH += ../../common
VPATH += ../../common
INCLUDEPATH += /usr/include/lapacke #this was needed on CentOS
DEFINES += USE_LAPACK
LIBS += -llapack -llapacke
LIBS += -fopenmp -lfftw3 -lfftw3_threads
#OPENMP
!macx {
  QMAKE_LFLAGS += -fopenmp
  QMAKE_CXXFLAGS += -fopenmp
}

