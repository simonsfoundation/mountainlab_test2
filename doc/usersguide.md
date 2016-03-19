# MountainLab Users Guide

## Concepts and Conventions

The **raw dataset** is assumed to be a $M\times N$ array of voltage measurements, where $M$ is the number of channels (electrodes) and $N$ is the number of timepoints. Unless otherwise specified, event times are in index units.

The **sample rate** is the rate at which the original data was acquired. The samplerate variable is always in units of Hz. So samplerate=30000 means that timepoint 30,000 occurs at acquisition time 1 second.

Generally speaking, we use the following conventions:

* M -- number of channels or electrodes
* N -- number of timepoints in the dataset
* K -- number of clusters
* L -- number of events
* T -- length of a clip (in timepoints)

Times, channels, and cluster labels are referenced using **1-based indexing**. Thus the first channel is channel $1$ and the first timepoint is timepoint $1$, and so on.

The **center of a clip** of size $T$ is always at the integer part of $(T+1)/2$ using 1-based indexing.

The raw and preprocessed datasets ($M\times N$ arrays) are referred to as **timeseries**.

The output of sorting is stored in a "**firings**" matrix of dimension $R\times L$ where $R\geq 3$, where $L$ is the number of events. Thus each column corresponds to a detected firing event. Information in the rows of the firings matrix are defined as follows:

* **First row (optional)** -- Primary channel number of the spike type, e.g., the channel where the average spike shape has the largest peak. If this information is not provided (by the sorter) this row may be filled with zeros.
* **Second row (mandatory)** -- Event times, or timepoints where the events were detected. These may have a fractional component.
* **Third row (mandatory)** -- Integer labels assigning membership to a cluster or spike type. A value of zero indicates a non-classified (or outlier) event.
* **Fourth row (optional)** -- Peak amplitudes.
* **Fifth row (optional)** -- Outlier scores. The larger the score, the more outlier-ish.
* **Sixth row and up (optional))** -- Unused for now.

## File I/O

All matrices and arrays are stored in .mda file format. This is an *almost pure* binary format containing a minimalistic header containing information about the data storage type, number of dimensions, and dimension sizes. More information on this file format is available [here](http://magland.github.io/html/2015-07-31-mda-format.html).

In Matlab, MDA files may be read/written using the following functions:
* readmda.m -- read array from file
* writemda32.m -- write array to file (32-bit floating point)
* writemda64.m -- write array to file (64-bit floating point)

In C++, the following classes may be used:
* Mda -- in-memory representation of multi-dimensional array
* DiskReadMda -- read-only access to a .mda file
* DiskWriteMda -- write-only access to a .mda file

The latter two classes are convenient for manipulating huge datasets such as long acquisitions with hundreds of channels.

## Processors

All data processing is performed using *processors* (aka subroutines or functions). Each processor acts on a set of input arrays and input parameters and outputs a set of output arrays. For example, the *bandpass_filter* processor acts on a timeseries and produces a new (filtered) time series. At this point, there are three types of processors:

* **In-memory processors** such as functions in MATLAB, which act on arrays that are already loaded into memory and return newly allocated arrays.
* **Command-line processors** which act on input files and produce output files (usually .mda format). These are programmed in C++ (see the *mountainsort* subproject).
* **Matlab wrappers** that make system calls to the command-line processors. Again, these act on files, but may be incorporated into MATLAB scripts.

The fundamental processing routines (e.g., bandpass_filter) have all three implementations, and so the user may select which to use, based on the context. For example, simple demo scripts may simply use in-memory MATLAB functions, whereas Matlab wrappers should be used when the datasets are prohibitively large. Command-line processors are convenient on computing clusters or on machines without MATLAB.

## Processing Scripts

Sorting algorithms are implemented using processing scripts which combine a number of processors in a pipeline. Just as there are three types of processors, there are generally three ways to write a sorting pipeline. The simplest way is to use in-memory MATLAB functions as follows:

```matlab
Y = readmda('raw.mda');
Yf = ms_filter(Y,o_filter);
Yw = ms_whiten(Yf);
times = ms_detect3(Yw,o_detect);
clips = ms_extract_clips2(Yf,times,clip_size);
features = ms_event_features(clips,num_features);
labels = isosplit2(features); %clustering
firings = [0*times; times; labels];
writemda64(firings,'firings.mda');
```
User-specified parameters are specified in o_filter, o_detect, clip_size, and num_features.

The same procedure may be scripted using Matlab wrappers to the command-line calls to the mountainsort executable:

```matlab
mscmd_filter('raw.mda','filt.mda',o_filter);
mscmd_whiten('filt.mda','white.mda');
mscmd_detect3('white.mda','detect.mda',o_detect);
mscmd_cluster('detect.mda','firings.mda');
```
Note that these codes are provided to illustrate the concepts of processing pipelines and will not necessarily run due to changes in the API. For more detailed sorting examples see the demo directories.

A third way to specify processing scripts is to use bash scripts with special syntax interpreted by mountainsort. In this case, Matlab is not required. Usage of these **.msh** scripts is described below.
