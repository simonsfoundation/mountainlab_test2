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
  
  # The following is to get the directory of the script, even resolving symlinks!!
  SOURCE="${BASH_SOURCE[0]}"
  while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
    DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
    SOURCE="$(readlink "$SOURCE")"
    [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
  done
  DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
  #################################################################################

  echo "$DIR"
  echo ""
  echo ""
  echo ""
  echo "****************** Starting $1 *********************"
  tmux kill-session -t $1
  tmux new -d -s $1 "cd $DIR/$1 ; ((nodejs $1.js) || (node $1.js)) ; bash" # handle both cases for node on the system (1) nodejs (2) node
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
