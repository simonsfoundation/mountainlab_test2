#!/bin/bash

echo "Compiling mountainbrowser"
cd mountainbrowser/src
qmake
make -j 8
cd ../..


