#!/bin/bash

main()
{
  start_it mlproxy
  start_it mdaserver
  start_it mbserver
  start_it mpserver
}

start_it()
{
  echo ""
  echo ""
  echo ""
  echo "****************** Starting $1 *********************"
  tmux kill-session -t $1
  tmux new -d -s $1 "cd $1 ; node $1.js ; bash"
  sleep 0.5
  tmux capture-pane -t $1
  tmux show-buffer | sed -e :a -e '/^\n*$/{$d;N;};/\n$/ba' # This new lines at end
  sleep 0.5
}

main
