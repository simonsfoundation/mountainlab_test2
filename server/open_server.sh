#!/bin/bash

if [ -z $1 ]
then
  echo "First argument should be name of server, eg, mdaserver"
  exit -1
fi

cd $1
tmux attach -t $1 || tmux new -s $1
