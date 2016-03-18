#!/bin/bash

# Compile mountainsort/cpp
echo "Compiling mountainsort"
cd mountainsort/src
qmake
make -j 8
cd ../..


