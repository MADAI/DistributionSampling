#!/bin/sh

# DistributionSamplingTest
# This is a little test suite for the MADAI Distribution Sampling Library

SRC_DIR=$(cd "$(dirname "$0")/.."; pwd;)

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
	[ "$DISPLAY" ] || { echo printfilelink ; return; }
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
	command -v sysctl > /dev/null && {
		sysctl -n hw.ncpu 2> /dev/null && return; }
	# Insert other logic here.
	echo 1; ## default value
}

## Build Function.  Returns 0 if build is successful.
build() (
	## this is the build script
	require mktemp make cmake
	cd "$(mktemp -d /tmp/"$(id -un)"_madai_XXXXXX)"
	BUILD_TYPE="$1" # Debug or Release
	INSTALL_PREFIX="$2"
	DEFAULT_PREFIX="/tmp/$(id -un)/local"
	#If Boost_INCLUDE_DIR is not /usr/include, set this
	if [ "$Boost_INCLUDE_DIR" ]; then
		BOOST_OPT="-DBoost_INCLUDE_DIR:PATH=${Boost_INCLUDE_DIR}"
	fi
	#If EIGEN3_INCLUDE_DIR is not /usr/include/eigen3, set this
	if [ "$EIGEN3_INCLUDE_DIR" ]; then
		EIGEN3_OPT="-DEIGEN3_INCLUDE_DIR:PATH=${EIGEN3_INCLUDE_DIR}"
	fi
	try cmake --quiet \
		-DCMAKE_INSTALL_PREFIX:PATH="${INSTALL_PREFIX:-$DEFAULT_PREFIX}" \
		-DCMAKE_BUILD_TYPE:STRING="${BUILD_TYPE}" \
		-DBUILD_TESTING:BOOL=1 \
		-DBUILD_DOCUMENTATION:BOOL=1 \
		-DUSE_OPENMP:BOOL=0 \
		-DUSE_GPROF:BOOL=0 \
		-DABORT_ON_COMPILER_WARNINGS:BOOL=1 \
		"$BOOST_OPT" "$EIGEN3_OPT" \
		"$SRC_DIR"
	try nice make -j"${NPROC:-$(get_nproc)}" all
	try nice make Documentation
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
	if ! command -v KWStyle > /dev/null; then return 0; fi
	TMP="$(mktemp /tmp/"$(id -un)"_madai_XXXXXX)"
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

parabolic_test() (
	NUMBER_OF_SAMPLES=${1:-10000}
	if [ "$2" ]; then
		MODE=interact
	else
		MODE=emulate
	fi
	PARALLEL_SAMPLING=$3
	PARABEXDIR="${SRC_DIR}/tutorial/parabolic_example"
	if [ '(' ! -d "$SRC_DIR" ')' -o ! -d "$PARABEXDIR" ] ; then
		echo "please set the SRC_DIR environment variable."
		return 1 ; fi
	require mktemp python
	# check to see that all of these got installed intp the PATH
	require madai_catenate_traces madai_launch_multiple_madai_generate_trace
	require madai_set_variable madai_print_default_settings
	require madai_generate_training_points madai_pca_decompose
	require madai_train_emulator

	TMPDIR="$(mktemp -d /tmp/"$(id -un)"_madai_XXXXXX)"
	# keep a list of tmpdirs to delete later
	echo "rm -Rf \"${TMPDIR}\"" >> /tmp/dsllist.sh
	try cd "$TMPDIR"

	try cp "${PARABEXDIR}/observable_names.dat" .
	try cp "${PARABEXDIR}/parameter_priors.dat" .
	cat > ./experimental_results.dat <<-EOF
	MEAN_X		 1.14		   0.1
	MEAN_X_SQUARED 2.77634418605  0.1
	MEAN_ENERGY	3.4925		 0.1
	# HELLO WORLD
	EOF
	try madai_print_default_settings > ./settings.dat
	try madai_set_variable . VERBOSE 1

	if [ "$MODE" = "interact" ] ; then
		try madai_set_variable . EXTERNAL_MODEL_EXECUTABLE \
			"${PARABEXDIR}/parabolic_interactive.py"
	else
		try madai_set_variable . EMULATOR_TRAINING_ALGORITHM exhaustive_geometric_kfold_common
		try madai_set_variable . GENERATE_TRAINING_POINTS_NUMBER_OF_POINTS 100
		try madai_set_variable . GENERATE_TRAINING_POINTS_USE_MAXIMIN 1
		try madai_generate_training_points . > /dev/null
		try python "${PARABEXDIR}/parabolic_evaluate.py" \
			./model_output/run* > /dev/null
		try madai_pca_decompose .

		try madai_set_variable . EMULATOR_SCALE 0.025
		try madai_train_emulator .
	fi

	try madai_set_variable . MCMC_USE_MODEL_ERROR 0
	try madai_set_variable . MCMC_NUMBER_OF_BURN_IN_SAMPLES 200

	if [ "$PARALLEL_SAMPLING" ] ; then
		NPROC=${NPROC:-$(get_nproc)}
		try madai_set_variable . SAMPLER_NUMBER_OF_SAMPLES \
			$(( $NUMBER_OF_SAMPLES / $NPROC ))
		try time madai_launch_multiple_madai_generate_trace . $NPROC output
		try madai_catenate_traces ./trace/output_*.csv > ./trace/output.csv
		rm ./trace/output_*.csv
	else
		try madai_set_variable . SAMPLER_NUMBER_OF_SAMPLES \
			$NUMBER_OF_SAMPLES
		try time -p nice madai_generate_trace . output.csv
	fi

	test -d "./model_output" && rm -r ./model_output

	if [ "$GNUPLOT_COMMAND" ] || {
			command -v gnuplot > /dev/null; } ; then
		SUF="${GNUPLOT_OUTPUT_SUFFIX:-pdf}"
		OUTFILE="${PWD}/$(getcommit "${SRC_DIR}")_${MODE}.${SUF}"
		try madai_gnuplot_scatterplot_matrix ./trace/output.csv "$OUTFILE" \
			parameter_priors.dat 50
		$(getopencmd) "$OUTFILE"
	else
		printfilelink ./trace/output.csv
	fi
	return 0
)

######################
PARALLEL_SAMPLING='1'
NUMBER_OF_SAMPLES=100000
NPROC=''
######################
INSTALL_PREFIX="${PREFIX:-/tmp/$(id -un)/local}"
PATH="${INSTALL_PREFIX}/bin:${PATH}"
try build "Release"
try build "Debug" "$INSTALL_PREFIX"
stylecheck ## can't 'try' because we don't pass the stylecheck
try parabolic_test "$NUMBER_OF_SAMPLES" ""  "$PARALLEL_SAMPLING"
try parabolic_test "$NUMBER_OF_SAMPLES" "1" "$PARALLEL_SAMPLING"
exit 0
