# ECWT_train1

## Preliminaries

### Dependencies

The C++ software here will compile under Visual Studio 2019 (VS2019), as it contains a Windows GUI. It also uses the VS2019 `long long` (`int`) type, which has 64 bits, to ensure the accuracy of certain calculations. Some features of C++ 20 are used, so the property ISO C++ 20 Standard (/std:c++20) needs to be set (follow C/C++ --> Language --> from the project property page in VS2019).

A further dependency is on the matrix manipulation code written by Gaël Guennebaud and Benoït Jacob *et al* and documented on https://eigen.tuxfamily.org, which needs to be downloaded and placed on the relevant path.

### Other non-code files

`description1.mp4` contains a clip showing the set-up and execution of the app, using sample data from https://zenodo.org/record/3519213#.Yel-TmjP3IU (accelerometry data collected by Alexandros Papadopoulos, Konstantinos Kyritsis, Sevasti Bostantjopoulou, Lisa Klingelhoefer, Kallol Ray Chaudhuri and Anastasios Delopoulos).

## Description

We wish to implement software based on the algorithms in the author's University of York EngD thesis. The application described here implements phase one training - building a library of "shapes" - paths in three-dimensional acceleration space, recorded using triaxal accelerometers. These paths are encoded as three piecewise polynomial wavelets in this space, allowing the advantages of the scale-invariance of wavelets to be used. This means that motions executed more rapidly or more slowly can be represented by the same shape.

More precisely, we use Equivalence Classes of Wavelet Triplets (ECWTs)

## ECWTs and Parkinson's Disease
