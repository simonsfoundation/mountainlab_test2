#!/bin/bash
echo "HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH"
sleep 5
echo "IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII"
cp $1 $2
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
