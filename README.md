# ECWT_train1

## Preliminaries

### Dependencies

The C++ software here will compile under Visual Studio 2019 (VS2019), as it contains a Windows GUI. It also uses the VS2019 `long long` (`int`) type, which has 64 bits, to ensure the accuracy of certain calculations.

A further dependency is on the matrix manipulation code written by Gaël Guennebaud and Benoït Jacob *et al* and documented on https://eigen.tuxfamily.org, which needs to be downloaded and placed on the relevant path.

### Other non-code files

## Description

We wish to implement software based on the algorithms in the author's University of York EngD thesis. The application described here implements phase one training - building a library of "shapes" - paths in three-dimensional acceleration space, recorded using triaxal accelerometers. These paths are encoded as three piecewise polynomial wavelets in this space, allowing the advantages of the scale-invariance of wavelets to be used. This means that motions executed more rapidly or more slowly can be represented by the same shape.

More precisely, we use Equivalence Classes of Wavelet Triplets (ECWTs)

## ECWTs and Parkinson's Disease
