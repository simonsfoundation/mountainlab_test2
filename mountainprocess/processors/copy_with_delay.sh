#!/bin/bash
sleep 2
cp $1 $2
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi