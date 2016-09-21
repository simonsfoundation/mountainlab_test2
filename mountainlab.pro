TEMPLATE = subdirs

# usage:
# qmake
# qmake "COMPONENTS = mountainview mountainbrowser"

#COMPONENTS = mdachunk mdaconvert mountainoverlook mountainprocess mountainview mountainbrowser prv

isEmpty(COMPONENTS) {
    COMPONENTS = mdachunk mdaconvert mountainoverlook mountainprocess mountainsort mountainview mountaincompare prv
    #we intentionally exclude mountainbrowser -- use: qmake "COMPONENTS = mountainbrowser"
}

CONFIG += ordered

SUBDIRS += mlcommon/src/mlcommon.pro

defineReplace(ifcomponent) {
  contains(COMPONENTS, $$1) {
    message(Enabling $$1)
    return($$2)
  }
  return("")
}

SUBDIRS += $$ifcomponent(mdachunk,labcomputer/mdachunk/src/mdachunk.pro)
SUBDIRS += $$ifcomponent(mdaconvert,mdaconvert/src/mdaconvert.pro)
SUBDIRS += $$ifcomponent(mountainbrowser,mountainbrowser/src/mountainbrowser.pro)
SUBDIRS += $$ifcomponent(mountainoverlook,mountainoverlook/src/mountainoverlook.pro)
SUBDIRS += $$ifcomponent(mountainprocess,mountainprocess/src/mountainprocess.pro)
SUBDIRS += $$ifcomponent(mountainsort,mountainsort/src/mountainsort.pro)
SUBDIRS += $$ifcomponent(mountainview,mountainview/src/mountainview.pro)
SUBDIRS += $$ifcomponent(mountaincompare,mountaincompare/src/mountaincompare.pro)
SUBDIRS += $$ifcomponent(prv,prv/src/prv.pro)

CONFIG(debug, debug|release) { SUBDIRS += tests }
