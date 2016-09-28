#!/bin/bash

# Before running this command, set up the web server using the instructions in the README.md

# Pass the absolute data directory path in as the first argument.

# Edit the configuration in prvfileserver.user.json. You can start by copying prvfileserver.default.json to prvfileserver.user.json.

# This script may seem crazy, but it is necessary to do this to achieve that the user
# inside the container matches exactly the current user (including uid!)

abs_data_directory=$1

if [ -z "$abs_data_directory" ];
then
  echo "You must specify the absolute data directory path as the first argument."
  exit -1
fi

shift #consume the first argument -- pass the rest to prvfileserver.js

# Remove any previously running containing with this name
sudo docker kill prv_container
sudo docker rm -f prv_container

# This is the command we will execute in the container
cmd="nodejs /base/prvfileserver/prvfileserver.js $abs_data_directory"

# We need to map the directory where the data are
args="--name=prv_container --net=host --pid=host -v $abs_data_directory:$abs_data_directory -v /tmp:/tmp"
if [ -f "$PWD/prvfileserver.user.json" ];
then
  # if it exists, map the prvfileserver.user.json file. Note that the prvfileserver.default.json file is copied over during build
  args="$args -v $PWD/prvfileserver.user.json:/base/prvfileserver.user.json"
fi

# Here's the crazy-ish but necessary way to do it
args="$args -t prv /bin/bash -c \"useradd -u $UID $USER;su - $USER -c '$cmd'\""

echo sudo docker run $args $@
/bin/bash -c "sudo docker run $args $@"
