FROM magland/ubuntu-qt5-nodejs

RUN apt-get install -y libfftw3-dev

RUN apt-get install -y octave

RUN apt-get install -y git nano htop

ADD . /home/mluser/dev/mountainlab
WORKDIR /home/mluser/dev/mountainlab
RUN ./compile_components.sh default

ENV PATH /home/mluser/dev/mountainlab/bin:$PATH
