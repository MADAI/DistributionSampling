#!/bin/sh

## Run with PATH set correctly:
##
##   export PATH=${PATH}:${HOME}/local/bin

for COMMAND in madai_change_setting madai_change_setting; do
	if ! command -v "$COMMAND" > /dev/null; then
		echo "${COMMAND} not found" >&1
		exit 1
	fi
done

CUP_DIR="$(cd "$(dirname "$0")"; pwd)"
DIR='.'

madai_change_setting "$DIR" VERBOSE 1
madai_change_setting "$DIR" SAMPLER PercentileGrid
madai_change_setting "$DIR" SAMPLER_NUMBER_OF_SAMPLES 90000
madai_change_setting "$DIR" SAMPLER_INACTIVE_PARAMETERS_FILE \
                          "${CUP_DIR}/inactive_parameters_2d.dat"
madai_change_setting "$DIR" EXTERNAL_MODEL_EXECUTABLE "${CUP_DIR}/cup.py"
madai_change_setting "$DIR" EXPERIMENTAL_RESULTS_FILE \
                          "${CUP_DIR}/experimental_results.dat"

nice madai_generate_trace "$DIR" "grid.csv"

