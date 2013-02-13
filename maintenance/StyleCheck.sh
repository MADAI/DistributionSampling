#!/bin/bash

# To use, run this from the top level of the DistributionSampling
# library source directory.

script_path="`dirname \"$0\"`"

files=`ls -1 src/*.h src/*.cxx`

for file in $files; do KWStyle -v $file -xml ${script_path}/KWStyle.xml | sed '/^$/d'; done


