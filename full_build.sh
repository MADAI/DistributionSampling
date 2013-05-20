#!/bin/sh

# This script downloads all dependencies, puts them in a temporary
# build location, configures and builds the DistributionSampling
# library, and installs it in the directory specified by the required
# command-line argument. This script must be run from within the
# DistributionSampling source directory root.
#
# WARNING: This script does not install Eigen and boost, so building
# against the Distribution Sampling library installed by this script
# may fail if headers in the Distribution Sampling library are
# included which themselves include Eigen and Boost.


BOOST_URL='http://sourceforge.net/projects/boost/files/latest/download?source=files'
EIGEN3_URL='http://bitbucket.org/eigen/eigen/get/3.1.3.tar.bz2'

Boost_INCLUDE_DIRS="/usr/include" # Change this if you already have
                                  # Boost somewhere else.
EIGEN3_INCLUDE_DIR="/usr/include/eigen3" # Change this if you already
                                         # have Eigen3 somewhere else.

die() { echo "$@" >&2; exit 1; }

require() {
    for arg; do
        command -v "$arg" >/dev/null 2>&1 || \
            die "${arg} is required but it is not installed. Exiting."
    done
}

if [ -z "$1" ] ; then
    echo "Usage:"
    echo "  $0 INSTALL_PREFIX"
    echo "Examples:"
    echo "  $0 \"\${HOME}/local\""
    echo "  $0 \"\${HOME}/madai\""
    echo "  $0 /usr/local"
    exit 1
fi

# Check prerequisites (curl, CMake, make )
require curl
require cmake
require make

cd `dirname "$0"`
SRC_DIR="`pwd`"
BUILD_DIR="/tmp/DSL"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

if ! [ -d "${Boost_INCLUDE_DIRS}/boost" ] ; then
    echo "Downloading Boost"
    echo "  $BOOST_URL"
    curl --location "$BOOST_URL" | tar --extract --bzip2 || \
        die "Error downloading Boost"
    mv `find . -maxdepth 1 -name boost\* -type d` boost || \
        die "Error finding Boost"
    Boost_INCLUDE_DIRS="`pwd`/boost"
fi

if ! [ -d "${EIGEN3_INCLUDE_DIR}/Eigen" ] ; then
    echo "Downloading Eigen"
    echo "  $EIGEN3_URL"
    curl --location "$EIGEN3_URL" | tar --extract --bzip2 || \
        die "Error downloading Eigen3"
    mv `find . -maxdepth 1 -name eigen\* -type d` eigen3 || \
        die "Error finding Eigen3"
    EIGEN3_INCLUDE_DIR="`pwd`/eigen3"
fi

INSTALL_PREFIX="$1"

mkdir -p DistributionSamplingBuild
cd DistributionSamplingBuild
echo "Running CMake"
cmake "$SRC_DIR" \
  -DCMAKE_INSTALL_PREFIX:PATH="${INSTALL_PREFIX}" \
  -DBoost_INCLUDE_DIR:PATH="$Boost_INCLUDE_DIRS" \
  -DEIGEN3_INCLUDE_DIR:PATH="$EIGEN3_INCLUDE_DIR" \
  -DCMAKE_BUILD_TYPE:STRING=Release \
  -DBUILD_TESTING:BOOL=0 \
  -DUSE_OPENMP:BOOL=0 \
  -DUSE_GPROF:BOOL=0  || die "Error in CMake"

echo "Running make"
make || die "Error in make"

echo "Running make install"
make install || {
    echo "Error in 'make install'." >&2
    echo "Try this:" >&2
    echo "  $ sudo make -C \"`pwd`\" install" >&2
    echo "  $ rm -rf \"$BUILD_DIR\"" >&2
    die
}

# Clean up
cd "$SRC_DIR"
rm -rf "$BUILD_DIR"

echo "SUCCESS"
