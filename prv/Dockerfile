############################################################
# prv
############################################################

# Set the base image to Ubuntu
FROM ubuntu:16.04

# Update the repository sources list
RUN apt-get update

# Install qt5
RUN apt-get install -y software-properties-common
RUN apt-add-repository ppa:ubuntu-sdk-team/ppa
RUN apt-get update
RUN apt-get install -y qtdeclarative5-dev
RUN apt-get install -y qt5-default qtbase5-dev qtscript5-dev make g++

# Install nodejs and npm
RUN apt-get install -y nodejs npm
RUN npm install ini extend

# Make the source directory
RUN mkdir -p /prv
WORKDIR /prv
RUN ln -s $PWD /base
ENV path /prv/bin:$PATH

# Compile prv C++
ADD src src
WORKDIR src
RUN qmake
RUN make -j 4
WORKDIR ..

# Add the source files for prvfileserver
ADD prv.json.default prv.json.default
ADD prvfileserver prvfileserver

#CMD nodejs prvfileserver/prvfileserver.js
