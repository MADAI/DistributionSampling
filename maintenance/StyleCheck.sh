#!/bin/bash

# To use, run this from the top level of the DistributionSampling
# library source directory.

kwstyle="KWStyle"
#kwstyle="${HOME}/build/KWStyle/KWStyle"

if (! which "$kwstyle" > /dev/null) ; then
	echo "KWStyle not found";
	exit 1
fi

script_path="`cd \`dirname \"$0\"\`; pwd`"

cd "${script_path}/.."

for file in src/*.h src/*.cxx; do
	"$kwstyle" -v "$file" -xml "${script_path}/KWStyle.xml" | sed '/^$/d';
done

for file in test/*.cxx; do
	"$kwstyle" -v "$file" -xml "${script_path}/KWStyleTest.xml" | sed '/^$/d';
done
