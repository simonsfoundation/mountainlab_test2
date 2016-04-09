#!/bin/bash

echo "Compiling mdachunk"
cd server/mdachunk/src
qmake
make -j 8
cd ../../..


