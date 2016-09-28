# Set the base image to Ubuntu
FROM magland/ubuntu-qt5-nodejs

MAINTAINER Jeremy Magland

# Install fftw and octave
RUN apt-get install -y libfftw3-dev octave

ADD . /home/mluser/dev/mountainlab
WORKDIR /home/mluser/dev/mountainlab
RUN ./compile_components.sh prv

ENV PATH /home/mluser/dev/mountainlab/bin:$PATH

##########################################
RUN ln -s /home/mluser/dev/mountainlab/prv /base
