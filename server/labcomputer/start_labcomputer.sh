#!/bin/bash

main()
{
  start_server mlproxy
  start_server mdaserver
  start_server mbserver
  start_server mpserver
  start_daemon mpdaemon
}

start_server()
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

start_daemon()
{
  echo ""
  echo ""
  echo ""
  echo "****************** Starting $1 *********************"
  tmux kill-session -t $1
  tmux new -d -s $1 "mountainprocess daemon-start ; bash"
  sleep 0.5
  tmux capture-pane -t $1
  tmux show-buffer | sed -e :a -e '/^\n*$/{$d;N;};/\n$/ba' # This new lines at end
  sleep 0.5
}

main
