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

INCLUDEPATH += utils core processors mda unit_tests 3rdparty isosplit

HEADERS += \
    utils/get_command_line_params.h \
    core/msprocessmanager.h \
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
    mda/diskwritemda.h \
    unit_tests/unit_tests.h \
    msprefs.h \
    processors/detect_processor.h \
    processors/detect.h \
    processors/whiten_processor.h \
    processors/whiten.h \
    utils/get_sort_indices.h \
    utils/eigenvalue_decomposition.h \
    utils/matrix_mda.h \
    processors/branch_cluster_v2_processor.h \
    processors/branch_cluster_v2.h \
    isosplit/isosplit2.h \
    isosplit/isocut.h \
    isosplit/jisotonic.h \
    utils/get_principal_components.h \
    processors/extract_clips.h \
    utils/msmisc.h \
    processors/remove_duplicate_clusters_processor.h \
    processors/remove_duplicate_clusters.h \
    processors/remove_noise_subclusters_processor.h \
    processors/remove_noise_subclusters.h \
    processors/compute_outlier_scores_processor.h \
    processors/compute_outlier_scores.h \
    processors/copy_processor.h \
    processors/mda2txt_processor.h \
    core/process_msh.h \
    processors/mask_out_artifacts_processor.h \
    processors/mask_out_artifacts.h \
    utils/get_pca_features.h

SOURCES += mountainsortmain.cpp \
    utils/get_command_line_params.cpp \
    core/msprocessmanager.cpp \
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
    mda/diskwritemda.cpp \
    unit_tests/unit_tests.cpp \
    processors/detect_processor.cpp \
    processors/detect.cpp \
    processors/whiten_processor.cpp \
    processors/whiten.cpp \
    utils/get_sort_indices.cpp \
    utils/eigenvalue_decomposition.cpp \
    utils/matrix_mda.cpp \
    processors/branch_cluster_v2_processor.cpp \
    processors/branch_cluster_v2.cpp \
    isosplit/isosplit2.cpp \
    isosplit/isocut.cpp \
    isosplit/jisotonic.cpp \
    utils/get_principal_components.cpp \
    processors/extract_clips.cpp \
    utils/msmisc.cpp \
    processors/remove_duplicate_clusters_processor.cpp \
    processors/remove_duplicate_clusters.cpp \
    processors/remove_noise_subclusters_processor.cpp \
    processors/remove_noise_subclusters.cpp \
    processors/compute_outlier_scores_processor.cpp \
    processors/compute_outlier_scores.cpp \
    processors/copy_processor.cpp \
    processors/mda2txt_processor.cpp \
    core/process_msh.cpp \
    processors/mask_out_artifacts_processor.cpp \
    processors/mask_out_artifacts.cpp \
    utils/get_pca_features.cpp

DISTFILES += \
    ../version.txt


#LAPACK
#On Ubuntu: sudo apt-get install liblapacke-dev
#On CentOS: sudo yum install lapack-devel.i686
INCLUDEPATH += /usr/include/lapacke #this was needed on CentOS
DEFINES += USE_LAPACK
LIBS += -llapack -llapacke

#FFTW
LIBS += -fopenmp -lfftw3 -lfftw3_threads

#OPENMP
QMAKE_LFLAGS += -fopenmp
QMAKE_CXXFLAGS += -fopenmp
#-std=c++11   # AHB removed since not in GNU gcc 4.6.3

#QJSON
HEADERS += 3rdparty/qjson.h
SOURCES += 3rdparty/qjson.cpp
INCLUDEPATH += 3rdparty/qjson
DEPENDPATH += 3rdparty/qjson
VPATH += 3rdparty/qjson
HEADERS += serializer.h serializerrunnable.h parser.h parserrunnable.h json_scanner.h json_parser.hh
SOURCES += serializer.cpp serializerrunnable.cpp parser.cpp parserrunnable.cpp json_scanner.cpp json_parser.cc


