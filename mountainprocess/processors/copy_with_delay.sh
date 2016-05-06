#!/bin/bash
echo "HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH"
sleep 2
echo "IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII"
cp $1 $2
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi