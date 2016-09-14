############################################################
# Dockerfile to set up MountainLab
# Based on Ubuntu
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

# Install git
RUN apt-get install -y git

# Install fftw and octave
RUN apt-get install -y libfftw3-dev
RUN apt-get install -y octave

# Make the user
RUN mkdir /home/magland
RUN groupadd -r magland -g 433 && \
useradd -u 431 -r -g magland -d /home/magland -s /sbin/nologin -c "Docker image user" magland && \
chown -R magland:magland /home/magland

USER magland
WORKDIR /home/magland
RUN echo "------------------------------------ $PWD"
RUN whoami

# Make the development directory
RUN mkdir dev
WORKDIR dev

# Clone the mountainlab repository
RUN git clone https://github.com/magland/mountainlab.git
WORKDIR mountainlab

# Compile mountainlab
RUN ./compile_components.sh default

# Set up environment variables
RUN echo "export PATH=/home/magland/dev/mountainlab/mountainprocess/bin:\$PATH" >> /home/magland/.bashrc
RUN echo "export PATH=/home/magland/dev/mountainlab/mountainview/bin:\$PATH" >> /home/magland/.bashrc
RUN echo "export PATH=/home/magland/dev/mountainlab/mountaincompare/bin:\$PATH" >> /home/magland/.bashrc
RUN echo "export PATH=/home/magland/dev/mountainlab/kron:\$PATH" >> /home/magland/.bashrc

# Now I'll put you in the directory of the examples
WORKDIR examples

