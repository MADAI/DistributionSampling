#!/bin/bash

# To use, run this from the top level of the ModelOptimization
# library source directory.

files=`ls -1 src/*.h src/*.cxx`

for file in $files; do KWStyle -v $file; done
