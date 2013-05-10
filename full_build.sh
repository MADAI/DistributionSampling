#!/bin/sh

# This script downloads all dependencies, puts them in a temporary
# build location, configures and builds the DistributionSampling
# library, and installs it in the directory specified by the required
# command-line argument. This script must be run from within the
# DistributionSampling source directory root.

BOOST_URL='http://sourceforge.net/projects/boost/files/latest/download?source=files'
EIGEN3_URL='http://bitbucket.org/eigen/eigen/get/3.1.3.tar.bz2'

die() { echo "$@"; exit 1; }

if [ -z "$1" ] ; then
    echo "Usage:"
    echo "  $0 INSTALL_PREFIX"
    echo "Examples:"
    echo "  $0 \"\${HOME}/local\""
    echo "  $0 \"\${HOME}/madai\""
    echo "  $0 /usr/local"
    exit 1
fi

SRC_DIR="`pwd`"
BUILD_DIR="/tmp/DSL"
mkdir -p ${BUILD_DIR}
pushd ${BUILD_DIR}

# Check prerequisites (curl, CMake, make )
command -v curl >/dev/null 2>&1 || { echo "curl is required but it is not installed. Exiting." >&2; exit 1; }
command -v cmake >/dev/null 2>&1 || { echo "cmake is required but it is not installed. Exiting." >&2; exit 1; }
command -v make >/dev/null 2>&1 || { echo "make is required but it is not installed. Exiting." >&2; exit 1; }

echo "Downloading Boost"
echo "  $BOOST_URL"
curl --location "$BOOST_URL" | tar --extract --bzip2 || die "Error downloading Boost"
mv `find . -maxdepth 1 -name boost\* -type d` boost || die "Error finding Boost"
Boost_INCLUDE_DIRS="`pwd`/boost"

echo "Downloading Eigen"
echo "  $EIGEN3_URL"
curl --location "$EIGEN3_URL" | tar --extract --bzip2 || die "Error downloading Eigen3"
mv `find . -maxdepth 1 -name eigen\* -type d` eigen3 || die "Error finding Eigen3"
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
popd
rm -rf ${BUILD_DIR}

echo "SUCCESS"
