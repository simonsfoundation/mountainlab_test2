INCLUDEPATH += $$PWD/include $$PWD/include/cachemanager
#jfm switched back to static lib
LIBS += $$PWD/lib/libmlcommon.a
#LIBS += -L$$PWD/lib -lmlcommon

QMAKE_CXXFLAGS8

unix:PRE_TARGETDEPS += $$PWD/lib/libmlcommon.so
