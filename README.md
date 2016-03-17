# MountainLab
MountainLab is a collection of tools for processing and visualization of electrophysiological recordings. It is being developed by Jeremy Magland and Alex Barnett at the Simons Center for Data Analysis (SCDA).

The software suite comprises the following:

* MountainSort -- Core C++ library for sorting of multi-electride recordings.

* MountainView -- Collection of Qt/C++ widgets for visualising generic electrophysiological datasets and sorting results.

* A Matlab toolset for processing and visualisation of spike sorting data and a set of Matlab wrappers to MountainSort and MountainView.

* (Future) A Python toolset for processing and visualisation of spike sorting data and a set of Python wrappers to MountainSort and MountainView.

## Licence

This is open source software released under the terms of the GPLv3. A commercial version is also available.

## Installation and Basic Usage

MountainView has been developed and tested in Linux (Ubuntu and CentOS), but should also run (with some expertise) on any operating sytem that supports Qt/C++ such as Windows or Mac OS. Much of the Matlab functionality is available without the need to install the C++ programs. The C++ programs may also run independently (without Matlab) from the command line.

General instructions for all operating systems:

* Install git and clone this repository.

* Install Qt5 and Lapack development libraries.

* Compile MountainSort by running qmake and make from a terminal inside the mountainlab/mountainsort/src directory.

* Compile MountainView by running qmake and make from a terminal inside the mountainlab/mountainview/src directory.

* (Optional) From within Matlab navigate to mountainlab/matlab and run ms_setup_path.m.

Several demo command-line and Matlab scripts are found in /demo and /matlab/demo, respectively.

##Installation on Linux

Follow the above steps using the following hints along the way.

Installation of git:

Installation of Qt5:

Installation of Lapack:

Compilation:

Opening Matlab:
> matlab

##Methodology Used

References to manuscripts go here.

##Philosophy

* As much as possible spike sorting should be automatic to avoid user bias, enable validation, and facilitate processing of large amounts of data.

* Sorting methodology should be transparent and involve a minimumal set of adjustable parameters.

* Visualization tools should be independent of sorting methodology allowing results from multiple algorithms to be compared.



