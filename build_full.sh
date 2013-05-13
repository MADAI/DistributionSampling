#!/bin/sh

# This script downloads all dependencies, puts them in a temporary
# build location, configures and builds the DistributionSampling
# library, and installs it in the directory specified by the required
# command-line argument.

BOOST_URL='http://sourceforge.net/projects/boost/files/latest/download?source=files'
EIGEN3_URL='http://bitbucket.org/eigen/eigen/get/3.1.3.tar.bz2'

die() { echo "$@" >&2; exit 1; }
check_command() { command -v "$1" >/dev/null 2>&1 ; }

if [ -z "$1" ] ; then
    echo "Usage:" >&2;
    echo "  $0 INSTALL_PREFIX" >&2;
    echo "Examples:" >&2;
    echo "  $0 \"\${HOME}/local\"" >&2;
    echo "  $0 \"\${HOME}/madai\"" >&2;
    echo "  $0 /usr/local" >&2;
    exit 1
fi

# Check prerequisites (curl, CMake, make )
check_command curl  || \
	die "curl is required but it is not installed. Exiting."
check_command cmake || \
	die "cmake is required but it is not installed. Exiting."
check_command make  || \
	die "make is required but it is not installed. Exiting."

cd "`dirname "$0"`"
SRC_DIR="`pwd`"
BUILD_DIR="/tmp/DSL"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "Downloading Boost"
echo "  $BOOST_URL"
curl --location "$BOOST_URL" | tar --extract --bzip2 || \
	die "Error downloading Boost"
mv `find . -maxdepth 1 -name boost\* -type d` boost || \
	die "Error finding Boost"
Boost_INCLUDE_DIRS="`pwd`/boost"

echo "Downloading Eigen"
echo "  $EIGEN3_URL"
curl --location "$EIGEN3_URL" | tar --extract --bzip2 || \
	die "Error downloading Eigen3"
mv `find . -maxdepth 1 -name eigen\* -type d` eigen3 || \
	die "Error finding Eigen3"
EIGEN3_INCLUDE_DIR="`pwd`/eigen3"

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
make install || die "Error in 'make install'"

# Clean up
rm -rf ${BUILD_DIR}

echo "SUCCESS"
