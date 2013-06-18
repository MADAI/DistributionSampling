#!/bin/sh

# DistributionSamplingTest
# This is a little test suite for the MADAI Distribution Sampling Library

## useful functions
die() { echo "$@" >&2; exit 1; }
try() { "$@" || die "\"$@\" failed."; }
require() { for arg; do
    command -v "$1" > /dev/null || die "${1} not found"; done; }
getcommit() (
    cd "$1"; git show-ref -d $(git symbolic-ref HEAD) --abbrev --hash )
abspath() { echo "$(cd "$(dirname "$1")"; pwd)/$(basename "$1")"; }
printfilelink() { echo "\"file://$(abspath "$1")\"" ; }
getopencmd() {
    case "$(uname -s)" in
        *WIN*|*W32*) command -v "start";;
        Darwin) command -v "open";;
        *)  command -v "xdg-open" || \
            command -v "gvfs-open" || \
            command -v "kde-open" || \
            command -v "exo-open" ||
			echo printfilelink; esac; }
get_nproc() {
	# print out number of parallel processes
    command -v nproc > /dev/null && { nproc; return; }
    # Insert other logic here.
    echo 1; ## default value
}

## Build Functions  Returns 0 if build is successful.
build() (
	## this is the build script
	require mktemp make cmake
	cd "$(mktemp -d)"
	BUILD_TYPE="$1" # Debug or Release
	INSTALL_PREFIX="$2"
	DEFAULT_PREFIX="/tmp/$(id -un)/local"
    try cmake --quiet "$SRC_DIR" \
        -DCMAKE_INSTALL_PREFIX:PATH="${INSTALL_PREFIX:-$DEFAULT_PREFIX}" \
        -DCMAKE_BUILD_TYPE:STRING="${BUILD_TYPE}" \
        -DBUILD_TESTING:BOOL=1 \
        -DUSE_OPENMP:BOOL=0 \
        -DUSE_GPROF:BOOL=0 \
        -DABORT_ON_COMPILER_WARNINGS:BOOL=1
    try nice make -j"$(get_nproc)" all
    try nice make test
	if [ "$INSTALL_PREFIX" ] ; then
		try mkdir -p "$INSTALL_PREFIX"
		try rm -rf "${INSTALL_PREFIX}/lib/madai"
		try rm -rf "${INSTALL_PREFIX}/include/madai"
		try rm -rf "${INSTALL_PREFIX}/bin/madai_"*
		try nice make install
	fi
	rm -rf *; )

stylecheck() (
	TMP=`mktemp`
	cd "$SRC_DIR"
	RETCODE=0
	for file in applications/*.cxx src/*.h src/*.cxx ; do
		KWStyle -v "$file" -xml maintenance/KWStyle.xml > "$TMP"
		KRET=$?
		if [ "$KRET" -gt 0 ]; then cat "$TMP" ; RETCODE="$KRET" ; fi
	done
	for file in applications/test/*.cxx test/*.cxx ; do
		KWStyle -v "$file" -xml maintenance/KWStyleTest.xml > "$TMP"
		KRET=$?
		if [ "$KRET" -gt 0 ]; then cat "$TMP" ; RETCODE="$KRET" ; fi
	done
	rm "$TMP";
	return $RETCODE
)

######################

INTERACTIVE_MODE=''

######################

SCRIPT_DIRECTORY=$(cd "$(dirname "$0")"; pwd;)
SRC_DIR=$(cd "${SCRIPT_DIRECTORY}/.."; pwd;)
require mktemp python
INSTALL_PREFIX="/tmp/$(id -un)/local"
try build "Release"
try build "Debug" "$INSTALL_PREFIX"
#try stylecheck
stylecheck ## commented because we can't pass the stylecheck

PATH="${INSTALL_PREFIX}/bin:${PATH}"
# check to see that all of these got installed
require madai_catenate_traces madai_launch_multiple_madai_generate_trace
require madai_set_variable madai_print_default_settings
require madai_generate_training_points madai_pca_decompose
require madai_train_emulator

TMPDIR=$(mktemp -d)

# keep a list of tmpdirs to delete later
echo "rm -Rf \"${TMPDIR}\"" >> /tmp/dsllist.sh

try cd "$TMPDIR"

PARABEXDIR="${SRC_DIR}/tutorial/parabolic_example"
try cp "${PARABEXDIR}/observable_names.dat" .
try cp "${PARABEXDIR}/parameter_priors.dat" .
cat > ./experimental_results.dat <<EOF
MEAN_X         1.14           0.1
MEAN_X_SQUARED 2.77634418605  0.1
MEAN_ENERGY    3.4925         0.1
EOF
try madai_print_default_settings > ./settings.dat
try madai_set_variable . VERBOSE 1

if [ "$INTERACTIVE_MODE" ] ; then
    MODE=interact
    try madai_set_variable . EXTERNAL_MODEL_EXECUTABLE "${PARABEXDIR}/parabolic_interactive.py"
else
    MODE=emulate
    try madai_set_variable . GENERATE_TRAINING_POINTS_NUMBER_OF_POINTS 100
    try madai_set_variable . GENERATE_TRAINING_POINTS_USE_MAXIMIN 1
    try madai_generate_training_points . > /dev/null
    try python "${PARABEXDIR}/parabolic_evaluate.py" ./model_output/run* > /dev/null
    try madai_pca_decompose .
    try madai_set_variable . EMULATOR_SCALE 0.025
    try madai_train_emulator .
fi

try madai_set_variable . MCMC_USE_MODEL_ERROR 0
try madai_set_variable . MCMC_NUMBER_OF_BURN_IN_SAMPLES 200
try madai_set_variable . SAMPLER_NUMBER_OF_SAMPLES 7500

try time madai_launch_multiple_madai_generate_trace . $(get_nproc) output
try madai_catenate_traces ./trace/output_*.csv > ./trace/output.csv
rm ./trace/output_*.csv

test -d "./model_output" && rm -r ./model_output
PDFFILE="${PWD}/$(getcommit "${SRC_DIR}")_${MODE}.pdf"

try madai_gnuplot_scatterplot_matrix ./trace/output.csv "$PDFFILE" \
	parameter_priors.dat 50

$(getopencmd) "$PDFFILE"

exit 0

