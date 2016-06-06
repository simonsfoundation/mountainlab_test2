QT += core
QT -= gui
#CONFIG += release
#CONFIG -= debug
CONFIG -= app_bundle #Please apple, don't make a bundle today :)

#CONFIG += c++11
QMAKE_CXXFLAGS = -std=c++11

DESTDIR = ../bin
OBJECTS_DIR = ../build
MOC_DIR=../build
TARGET = mountainsort
TEMPLATE = app

QMAKE_POST_LINK += cp $$PWD/../bin/mountainsort $$PWD/../../mountainprocess/processors/mountainsort.mp

INCLUDEPATH += utils core processors mda unit_tests 3rdparty isosplit ../../common/cachemanager

HEADERS += \
    core/msprocessmanager.h \
    core/mountainsort_version.h \
    core/msprocessor.h \
    processors/example_processor.h \
    processors/bandpass_filter_processor.h \
    processors/bandpass_filter0.h \
    unit_tests/unit_tests.h \
    msprefs.h \
    processors/detect_processor.h \
    processors/detect.h \
    processors/whiten_processor.h \
    processors/whiten.h \
    processors/branch_cluster_v2_processor.h \
    processors/branch_cluster_v2.h \
    isosplit/isosplit2.h \
    isosplit/isocut.h \
    isosplit/jisotonic.h \
    processors/extract_clips.h \
    processors/remove_duplicate_clusters_processor.h \
    processors/remove_duplicate_clusters.h \
    processors/compute_outlier_scores_processor.h \
    processors/compute_outlier_scores.h \
    processors/copy_processor.h \
    processors/mda2txt_processor.h \
    processors/mask_out_artifacts_processor.h \
    processors/mask_out_artifacts.h \
    processors/adjust_times_processor.h \
    processors/adjust_times.h \
    processors/fit_stage_processor.h \
    processors/fit_stage.h \
    processors/compute_templates.h \
    processors/compute_templates_processor.h \
    processors/mv_firings_filter_processor.h \
    processors/mv_firings_filter.h \
    processors/extract_clips_processor.h \
    processors/mv_subfirings_processor.h \
    ../../common/cachemanager/cachemanager.h \
    processors/extract_clips_features_processor.h \
    processors/compute_detectability_scores_processor.h \
    processors/compute_detectability_scores.h \
    processors/merge_labels_processor.h \
    processors/merge_labels.h \
    processors/filter_events.h \
    processors/filter_events_processor.h \
    processors/confusion_matrix.h \
    processors/extract_raw_processor.h \
    processors/merge_across_channels.h \
    processors/merge_across_channels_processor.h \
    processors/geom2adj_processor.h \
    processors/mv_compute_templates_processor.h \
    processors/mv_compute_templates.h \
    processors/create_multiscale_timeseries_processor.h \
    processors/create_multiscale_timeseries.h

SOURCES += \
    core/msprocessmanager.cpp \
    core/mountainsort_version.cpp \
    core/msprocessor.cpp \
    processors/example_processor.cpp \
    processors/bandpass_filter_processor.cpp \
    processors/bandpass_filter0.cpp \
    unit_tests/unit_tests.cpp \
    processors/detect_processor.cpp \
    processors/detect.cpp \
    processors/whiten_processor.cpp \
    processors/whiten.cpp \
    processors/branch_cluster_v2_processor.cpp \
    processors/branch_cluster_v2.cpp \
    processors/extract_clips.cpp \
    processors/remove_duplicate_clusters_processor.cpp \
    processors/remove_duplicate_clusters.cpp \
    processors/compute_outlier_scores_processor.cpp \
    processors/compute_outlier_scores.cpp \
    processors/copy_processor.cpp \
    processors/mda2txt_processor.cpp \
    processors/mask_out_artifacts_processor.cpp \
    processors/mask_out_artifacts.cpp \
    processors/adjust_times_processor.cpp \
    processors/adjust_times.cpp \
    processors/fit_stage_processor.cpp \
    processors/fit_stage.cpp \
    processors/compute_templates.cpp \
    processors/compute_templates_processor.cpp \
    processors/mv_firings_filter_processor.cpp \
    processors/mv_firings_filter.cpp \
    processors/extract_clips_processor.cpp \
    processors/mv_subfirings_processor.cpp \
    ../../common/cachemanager/cachemanager.cpp \
    processors/extract_clips_features_processor.cpp \
    processors/compute_detectability_scores_processor.cpp \
    processors/compute_detectability_scores.cpp \
    processors/merge_labels_processor.cpp \
    processors/merge_labels.cpp \
    processors/filter_events.cpp \
    processors/filter_events_processor.cpp \
    processors/confusion_matrix.cpp \
    processors/confusion_matrix_processor.cpp \
    processors/extract_raw_processor.cpp \
    processors/fit_stage_new.cpp \
    processors/merge_across_channels.cpp \
    processors/merge_across_channels_processor.cpp \
    processors/geom2adj_processor.cpp \
    processors/mv_compute_templates_processor.cpp \
    processors/mv_compute_templates.cpp \
    processors/create_multiscale_timeseries_processor.cpp \
    processors/create_multiscale_timeseries.cpp
SOURCES_NOCXX11 += \
    isosplit/isosplit2.cpp \
    isosplit/isocut.cpp \
    isosplit/jisotonic.cpp \



INCLUDEPATH += ../../common/commandlineparams
VPATH += ../../common/commandlineparams
HEADERS += commandlineparams.h
SOURCES += commandlineparams.cpp

INCLUDEPATH += ../../common/utils
VPATH += ../../common/utils
HEADERS += textfile.h taskprogress.h
SOURCES += textfile.cpp taskprogress.cpp

HEADERS += utils/get_sort_indices.h \
    utils/matrix_mda.h \
    utils/msmisc.h \
    utils/get_pca_features.h \
    utils/compute_templates_0.h \
    utils/eigenvalue_decomposition.h

SOURCES += utils/get_sort_indices.cpp \
    utils/matrix_mda.cpp \
    utils/get_pca_features.cpp \
    utils/compute_templates_0.cpp \
    utils/msmisc.cpp
SOURCES_NOCXX11 += utils/eigenvalue_decomposition.cpp

DEFINES += USE_REMOTE_MDA
DEFINES += USE_SSE2
INCLUDEPATH += ../../common/mda
DEPENDPATH += ../../common/mda
VPATH += ../../common/mda
HEADERS += remotereadmda.h diskreadmda.h diskwritemda.h usagetracking.h mda.h mdaio.h
SOURCES += remotereadmda.cpp diskreadmda.cpp diskwritemda.cpp usagetracking.cpp mda.cpp mdaio.cpp

HEADERS += utils/get_principal_components.h
SOURCES += utils/get_principal_components.cpp

INCLUDEPATH += ../../common
DEPENDPATH += ../../common
VPATH += ../../common
HEADERS += mlutils.h
SOURCES += mlutils.cpp

DISTFILES += \
    ../version.txt

CXXFLAGS_NOCXX11 = -c -fopenmp -O2 -Wall -W -D_REENTRANT -fPIC -DUSE_REMOTE_MDA -DUSE_SSE2 -DUSE_LAPACK -DQT_NO_DEBUG -DQT_CORE_LIB
nocxx11.name = nocxx11
nocxx11.input = SOURCES_NOCXX11
#cxx11.dependency_type = TYPE_C
nocxx11.variable_out = OBJECTS
nocxx11.output = ${QMAKE_VAR_OBJECTS_DIR}${QMAKE_FILE_IN_BASE}$${first(QMAKE_EXT_OBJ)}
nocxx11.commands = $${QMAKE_CXX} $${CXXFLAGS_NOCXX11}  $(INCPATH) ${QMAKE_FILE_IN} -o ${QMAKE_FILE_OUT} # Note the -O0
QMAKE_EXTRA_COMPILERS += nocxx11

#LAPACK
#On Ubuntu: sudo apt-get install liblapacke-dev
#On CentOS: sudo yum install lapack-devel.i686
INCLUDEPATH += /usr/include/lapacke #this was needed on CentOS
DEFINES += USE_LAPACK
LIBS += -llapack -llapacke

#FFTW
LIBS += -fopenmp -lfftw3 -lfftw3_threads

#OPENMP
!macx {
  QMAKE_LFLAGS += -fopenmp
  QMAKE_CXXFLAGS += -fopenmp
}
#-std=c++11   # AHB removed since not in GNU gcc 4.6.3

#tests
test {
    QT += testlib
    CONFIG += testcase
    TARGET = mountainsort_test
    DEPENDPATH += unit_tests
    SOURCES += unit_tests/testMda.cpp	\
	unit_tests/testMain.cpp	\
        unit_tests/testMdaIO.cpp \
        unit_tests/testBandpassFilter.cpp
    HEADERS += unit_tests/testMda.h \
        unit_tests/testMdaIO.h  \
        unit_tests/testBandpassFilter.h
} else {
    SOURCES += mountainsortmain.cpp
}
