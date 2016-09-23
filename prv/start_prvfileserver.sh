#!/bin/bash

# Before running this command, set up the web server using the instructions in the README.md

# Pass the absolute data directory path in as the first argument.

# Edit the configuration in prv.json. You can start by copying prv.json.default to prv.json.

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
sudo docker rm prv_container

# This is the command we will execute in the container
cmd="nodejs /base/prvfileserver/prvfileserver.js $abs_data_directory"

# We need to map the directory where the data are
args="--name=prv_container --net=host --pid=host -v $abs_data_directory:$abs_data_directory"
if [ -f "$PWD/prv.json" ];
then
  # if it exists, map the prv.json file. Note that the prv.json.file is copied over during build
  args="$args -v $PWD/prv.json:/base/prv.json"
fi

# Here's the crazy-ish but necessary way to do it
args="$args -t prv /bin/bash -c \"useradd -u $UID $USER;su - $USER -c '$cmd'\""

echo sudo docker run $args $@
/bin/bash -c "sudo docker run $args $@"
