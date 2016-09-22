# MountainLab

**Note: the documentation / installation instructions are out-dated and in progress. So please contact me (Jeremy) for help getting started. You may also be able to help us create the docs.**

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

When that succeeds, you can go right to the self-contained examples to test your installation.

> cd examples/003_kron_mountainsort

> ./001_generate_synthetic_data.sh

This will use matlab if you have it installed, otherwise it will use octave. It will generate 5 example synthetic datasets in an examples subdirectory, and the raw data are written to the BIGFILES subdirectory. Thus we begin following the principle of separating large files from their contents, as will be described in more detail.

> kron-run ms example1

This runs a standard mountainsort processing pipeline.

> kron-view results ms example1

This should launch the central viewer.

## Installing the prerequesites

Linux/Ubuntu or Debian is easiest. Mac and other Linux flavors should also work. Windows is not necessarily excluded.

The required packages are: 


[About the software](https://mountainlab.vbulletin.net/articles/22-about-mountainlab)

[Installation instructions](https://mountainlab.vbulletin.net/articles/19-installing-mountainlab)

[Forum](https://mountainlab.vbulletin.net/) -- including documentation and latest developments

## References

[Barnett, Alex H., Jeremy F. Magland, and Leslie F. Greengard. "Validation of Neural Spike Sorting Algorithms without Ground-truth Information." Journal of Neuroscience Methods 264 (2016): 65-77.](http://www.ncbi.nlm.nih.gov/pubmed/26930629) [Link to arXiv](http://arxiv.org/abs/1508.06936)

Magland, Jeremy F., and Alex H. Barnett. Unimodal clustering using isotonic regression: ISO-SPLIT. [Link to arXiv](http://arxiv.org/abs/1508.04841)

