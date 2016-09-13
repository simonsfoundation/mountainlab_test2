First compile and set up mountainlab

This includes having the following in your path:
mountainlab/mountainprocess/bin
mountainlab/mountainview/bin
mountainlab/mountaincompare/bin
mountainlab/kron

To generate the synthetic data, you must also have Octave or Matlab installed

001_synthesize_raw_data.sh - creates
	raw.mda, firings_true.mda, and waveforms.mda

002_sort.sh - performs sorting and writes files to output directory

003_view.sh - opens mountainview and shows the results of sorting

004_compare_with_ground_truth.sh - opens mountaincompare to compare the output with with the firings_true.mda