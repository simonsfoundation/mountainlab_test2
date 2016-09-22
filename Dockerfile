# Set the base image to Ubuntu
FROM ubuntu:16.04

# Update the repository sources list
RUN apt-get update

# Install gedit and other useful things
RUN apt-get install -y gedit nano git htop

# Install qt5
RUN apt-get install -y software-properties-common
RUN apt-add-repository ppa:ubuntu-sdk-team/ppa
RUN apt-get update
RUN apt-get install -y qtdeclarative5-dev
RUN apt-get install -y qt5-default qtbase5-dev qtscript5-dev make g++

RUN apt-get install -y libfftw3-dev

RUN apt-get install -y git

# Install nodejs and npm
RUN apt-get install -y nodejs npm
RUN npm install ini extend

ADD . /home/mluser/dev/mountainlab
WORKDIR /home/mluser/dev/mountainlab
RUN ./compile_components.sh default

ENV PATH /home/mluser/dev/mountainlab/bin:$PATH


