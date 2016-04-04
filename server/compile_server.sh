#!/bin/bash

# Compile mdachunk
echo "Compiling mountainsort"
cd mdachunk/src
qmake
make -j 8
cd ../..


