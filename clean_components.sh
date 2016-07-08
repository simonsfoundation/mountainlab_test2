#!/bin/bash

if [ -z "$1" ]; then
    echo "usage:"
    echo "./clean_components.sh default"
    echo "./clean_components.sh mountainview mountainbrowser"
    echo "example components: mdachunk mdaconvert mountainbrowser mountainoverlook mountainprocess mountainsort mountainview"
    exit 0
fi

if [ $1 == "default" ]
then
qmake
else
qmake "COMPONENTS = $@"
fi

make clean
