TEMPLATE = subdirs


isEmpty(COMPONENTS) {
    COMPONENTS = mvbrowser mvview mvsort
}

CONFIG += ordered


SUBDIRS += mlcommon/src/mlcommon.pro

defineReplace(mvcomponent) {
  contains(COMPONENTS, $$1) {
    message(Enabling $$1 component)
    return($${2}/src/$${2}.pro)
  }
  return ""
}

SUBDIRS += $$mvcomponent(mvbrowser, mountainbrowser)
SUBDIRS += $$mvcomponent(mvview, mountainview)
SUBDIRS += $$mvcomponent(mvsort, mountainsort)

