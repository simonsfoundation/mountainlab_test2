/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef isocut_h
#define isocut_h

/*
 * MCWRAP [ cutpoint[1,1] ] = isocut(X[1,N],threshold)
 * SET_INPUT N = size(X,2)
 * SOURCES isocut.cpp mda.cpp jisotonic.cpp
 * HEADERS isocut.h
 */
bool isocut(int N, double* cutpoint, double* X, double threshold);
bool isocut(int N, double* cutpoint, const double* X, double threshold, int minsize);
//return true if split is statistically significant

#endif
