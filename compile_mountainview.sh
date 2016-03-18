#!/bin/bash

# Compile mountainview
echo "Compiling mountainview"
cd mountainview/src
qmake
make -j 8
cd ../..


