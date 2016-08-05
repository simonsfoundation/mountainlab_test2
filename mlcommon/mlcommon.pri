INCLUDEPATH += $$PWD/include $$PWD/include/cachemanager
MLCOMMONLIB = $$PWD/lib/libmlcommon.a
LIBS += -L$$PWD/lib $$MLCOMMONLIB

unix:PRE_TARGETDEPS += $$MLCOMMONLIB

#The old version was as follows
#INCLUDEPATH += $$PWD/include $$PWD/include/cachemanager
#LIBS += $$PWD/lib/libmlcommon.a
#QMAKE_CXXFLAGS8
#unix:PRE_TARGETDEPS += $$PWD/lib/libmlcommon.so
