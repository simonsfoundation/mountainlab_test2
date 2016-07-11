/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 7/11/2016
*******************************************************/

#ifndef PCA_H
#define PCA_H

#include "mda.h"

// see info below
void pca(Mda& components, Mda& features, Mda& sigma, Mda& X, int num_features);

// same as pca, except input it X*X', and features are not computed (because how could they be?)
void pca_from_XXt(Mda& components, Mda& sigma, Mda& XXt, int num_features);

// get the whitening matrix as described below
void whitening_matrix_from_XXt(Mda& W, Mda& XXt);

void pca_unit_test();

/*
  pca: compute the top K principal components along with the corresponding feature vectors and
  singular values

  Input:
  X -- MxN matrix

  Output:
  C = components -- MxK, where K=num_features
  F = features -- KxN
  sigma = singular values -- Kx1

  components are normalized so that
  C'*C = eye(K,K)

  F = C'*X
  X is approximated by C*F = C*C'*X

  The components are eigenvectors of X*X':
  X*X'*C = C * diag(sigma)

  where sigma are the singular values associated with the top K components

  sigma(1)>=...>=sigma(K)

  if (K=M) then
  C'*C=C*C'=eye(M,M)
  X*X' = C * diag(sigma) * C' is the svd of X*X'

  The whitening matrix is:
  W = C * diag(sigma^(-1/2)) *C' = CDC'
  because
  (WX)*(WX)' = WXX'W' = CDC'XX'CDC' = C*D*diag(sigma)*D*C' = C*C' = eye(M,M)
*/

#endif // PCA_H
