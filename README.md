# MountainLab
MountainLab is a collection of processing, visualization, and validation tools for spike sorting of electrophysiological recordings. It is under development by Jeremy Magland and Alex Barnett at the Simons Center for Data Analysis (SCDA).

The software suite (in flux) comprises the following:

* MountainSort -- Core C++ library for sorting of multi-electride recordings.

* MountainView -- Collection of Qt/C++ widgets for visualizing generic electrophysiological datasets and sorting results.

* A Matlab toolset for processing, visualizing, and validating spike sorting data and a set of Matlab wrappers to MountainSort and MountainView.

## License

This is open source software released under the terms of the GPLv3. For alternative licensing options, please contact scdainfo@simonsfoundation.org.

## Installation and Basic Usage

MountainLab has been developed and tested in Linux (Ubuntu and CentOS), but should also run (with some expertise) on any operating sytem that supports Qt/C++ such as Windows or Mac OS X. Much of the Matlab functionality is available without needing to install the C++ programs. C++ programs also run independently (without Matlab) from the command line.

General instructions for all operating systems (see specifics below):

* Install git and clone this repository.

* Install Qt5 and Lapack development libraries.

* Compile MountainSort by running qmake and make from a terminal inside the mountainlab/mountainsort/src directory.

* Compile MountainView by running qmake and make from a terminal inside the mountainlab/mountainview/src directory.

* (Optional) From within Matlab navigate to mountainlab/matlab and run ms_setup_path.m.

Several demo command-line and Matlab scripts are found in /demo and /matlab/demo, respectively.

##Installation on Linux

Follow the above steps using the these hints along the way.

**Installation of git:**
```bash
Ubuntu/Debian: sudo apt-get install git
CentOS/Red Hat: sudo yum install git
```

**Installation of Qt5:**

[Download Qt5](https://www.qt.io/download-open-source/)
(Follow the instructions for installing the open source version)

**Installation of Lapack:**
```bash
Ubuntu/Debian: sudo apt-get install liblapack-dev
CentOS/Red Hat: sudo yum install lapack-devel.x86_64
```

**Compilation:**
```bash
cd mountainsuite/mountainsort/src
qmake
make -j 8
```
> The "-j 8" tells make to use multiple cores to do the compilation, because we believe in speed

**Opening Matlab:**
```
# First buy matlab
matlab
```

##References

[Barnett, Alex H., Jeremy F. Magland, and Leslie F. Greengard. "Validation of Neural Spike Sorting Algorithms without Ground-truth Information." Journal of Neuroscience Methods 264 (2016): 65-77.](http://www.ncbi.nlm.nih.gov/pubmed/26930629) [Link to arXiv](http://arxiv.org/abs/1508.06936)

Magland, Jeremy F., and Alex H. Barnett. Unimodal clustering using isotonic regression: ISO-SPLIT. [Link to arXiv](http://arxiv.org/abs/1508.04841)

