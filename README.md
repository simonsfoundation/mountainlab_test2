# MountainLab

MountainSort is spike sorting software developed by Jeremy Magland, Alex Barnett, and Leslie Greengard at the Simons Center for Data Analysis in close collaboration with Jason Chung and Loren Frank at UCSF department of Physiology. It is part of MountainLab, a general framework for data analysis and visualization.

MountainLab is being developed by Jeremy Magland and Witold Wysota.

The software comprises tools for processing electrophysiological recordings and for visualizing and validating the results.

## Installation and getting started

This is easiest if you are on Linux/Ubuntu, but should work on OS X and other flavors of Linux. Windows is also an operating system.

First, install the prerequisites (see below).

Next, clone this repository and compile:

> git clone https://github.com/magland/mountainlab.git

> cd mountainlab

> ./compile_components default

Add mountainlab/bin to your PATH environment variable. For example append

> export PATH=/path/to/mountainlab/bin:$PATH

to your ~/.bashrc file, and open a new terminal.

Next you can go right to the self-contained examples to test your installation.

> cd examples/003_kron_mountainsort

> ./001_generate_synthetic_data.sh

This will use matlab if you have it installed, otherwise it will use octave. It will generate 5 example synthetic datasets in an examples subdirectory, and the raw data are written to the BIGFILES subdirectory. Thus we begin following the principle of separating large files from their contents, as will be described in more detail.

> kron-run ms example1

This runs a standard mountainsort processing pipeline.

> kron-view results ms example1

This should launch the central viewer.

## Installing the prerequesites

Linux/Ubuntu or Debian is easiest. Mac and other Linux flavors should also work. Windows is not necessarily excluded.

The required packages are: Qt5 (v5.5 or later), FFTW, Octave, NodeJS

The most up-to-date procedure for installing these packages is reflected in the mountainlab/Dockerfile. But here are the current set of operations for Ubuntu 16.04 (let me know if they are out of date):

#### Qt5

If you are on a later version of Ubuntu (such as 16.04), you can get away with the installing Qt5 using the package manager. Otherwise, that method may not give a recent enough Qt5 version. In that case (for example if you are on 14.04, or using Mac or other Linux flavor), you should install Qt5 via qt.io website. Instructions are given in a section below.

> apt-get install -y software-properties-common

> apt-add-repository ppa:ubuntu-sdk-team/ppa

> apt-get update

> apt-get install -y qtdeclarative5-dev

> apt-get install -y qt5-default qtbase5-dev qtscript5-dev make g++

#### FFTW
> apt-get install -y libfftw3-dev

#### Octave
> apt-get install -y octave

#### NodeJS
> apt-get install -y nodejs npm

> npm install ini extend

## Installing Qt5 from qt.io

As mentioned above, if you are not using a later version of Ubuntu, you probably won't get a recent enough version from the package manager. In that case follow these directions to install a recent version of Qt5:

Go to [https://www.qt.io/download/|https://www.qt.io/download/] and click through that you want to install the open source version. Download and run the appropriate installer. Note that you do not need to set up a Qt account -- you can skip that step in the install wizard.

We recommend installing this in your home directory, which does not require admin privileges.

Once installed you will need to prepend the path to qmake to your PATH environment variable. On my system that is /home/magland/Qt/5.7/gcc_64/bin.

## How to run spike sorting

See the following article which explains how to actually perform spike sorting:

[https://mountainlab.vbulletin.net/articles/15-running-spike-sorting-using-mountainlab|https://mountainlab.vbulletin.net/articles/15-running-spike-sorting-using-mountainlab]

## Older information

[About the software](https://mountainlab.vbulletin.net/articles/22-about-mountainlab)

[Forum](https://mountainlab.vbulletin.net/) -- including documentation and latest developments

## References

[Barnett, Alex H., Jeremy F. Magland, and Leslie F. Greengard. "Validation of Neural Spike Sorting Algorithms without Ground-truth Information." Journal of Neuroscience Methods 264 (2016): 65-77.](http://www.ncbi.nlm.nih.gov/pubmed/26930629) [Link to arXiv](http://arxiv.org/abs/1508.06936)

Magland, Jeremy F., and Alex H. Barnett. Unimodal clustering using isotonic regression: ISO-SPLIT. [Link to arXiv](http://arxiv.org/abs/1508.04841)

