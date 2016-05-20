#!/bin/bash

REMOTE_HOST=$1
REMOTE_PORT=$2
LOCAL_PORT=$3
USER_NAME=$4

# $COMMAND is the command used to create the reverse ssh tunnel
COMMAND="ssh -N -f -R $REMOTE_PORT:localhost:$LOCAL_PORT $USER_NAME@$REMOTE_HOST"

echo $COMMAND

# Is the tunnel up? Perform two tests:
# 1. Check for relevant process ($COMMAND)
pgrep -f -x "$COMMAND" > /dev/null 2>&1 || $COMMAND

# 2. Test tunnel by looking at "netstat" output on $REMOTE_HOST
ssh $USER_NAME@$REMOTE_HOST netstat -an | egrep "tcp.*:$REMOTE_PORT.*LISTEN"
ssh $USER_NAME@$REMOTE_HOST netstat -an | egrep "tcp.*:$REMOTE_PORT.*LISTEN" \
> /dev/null 2>&1

if [ $? -ne 0 ] ; then
echo "Tunnel not up. Killing and re-running."
pkill -f -x "$COMMAND"
$COMMAND
fi
