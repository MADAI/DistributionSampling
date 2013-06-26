Project: DistributionSampling
README.txt author: Cory Quammen <cquammen@cs.unc.edu>,
    Hal Canary <cs.unc.edu/~hal>
Last updated: 2013-06-26

*** BUILD INSTRUCTIONS FOR LINUX AND MAC ***

** PREREQUISITES **

-   A compiler.
    - "yum groupinstall "Development Tools" on Red Hat-based systems
    - "apt-get install build-essential" on Debian-based systems
    - XCode on MacOS 10.x systems.

-   CMake version 2.8 or greater
    (Available at http://www.cmake.org/cmake/resources/software.html)
    - "yum install cmake" on Red Hat-based systems
    - "apt-get install cmake" on Debian-based systems
    - "port install cmake" on Macintosh systems with macports installed

-   The Boost Library (http://www.boost.org/users/download/
    - "yum install boost-devel" on Red Hat-based systems
    - "apt-get install libboost-dev" on Debian-based systems
    - "port install boost" on Macintosh systems with macports installed

-   The Eigen3 Library (http://eigen.tuxfamily.org/)
    - "yum install eigen3-devel" on Red Hat-based systems
    - "apt-get install libeigen3-dev" on Debian-based systems
    - "port install eigen3" on Macports systems with macports installed

** CONFIGURING **

1). Create a build directory outside the git repository in which this
    file is located. For example, if the directory containing this
    README.txt file is located in

    $HOME/madai/stat/DistributionSampling

    A good build directory might be

    $HOME/madai/build/stat/DistributionSampling-build

2). Navigate to the build directory created in step 1.

3). In the instructions below, "$ " represents your shell prompt.

    Type

    $ cmake ".../stat/DistributionSampling" \
          -DBUILD_TESTING:BOOL="1" \
          -DCMAKE_BUILD_TYPE:STRING="Debug"

    where .../stat/DistributionSampling is an absolute or relative path
    to the DistributionSampling source directory.

    Using the example directories above, this command is

    $ cmake "$HOME/madai/stat/DistributionSampling" \
          -DBUILD_TESTING:BOOL="1" \
          -DCMAKE_BUILD_TYPE:STRING="Debug"

4). Type

    $ make

    The various source files will compile, producing two libraries
    libDistributionSampling.a and libmadaisys.a in a directory called
    "lib" in the root level of the build directory.

5.  (optional) Type

    $ ctest

    This runs the tests defined in the project.

6. (optional) Generate documentation by typing

    $ cmake "$HOME/madai/stat/DistributionSampling" \
          -DBUILD_DOCUMENTATION:BOOL="1"
    $ make Documentation

    This generates HTML documentation of the library with doxygen.

** CONFIGURATION OPTIONS **

ABORT_ON_COMPILER_WARNINGS (boolean)

    Tells the compiler to treat warnings as errors.

USE_OPENMP (boolean)

    Look for and use OpenMP if found. This parallelizes part of the
    sample generation code.

    WARNING: OpenMP on Mac OS X appears to be buggy.

USE_GPROF (boolean)

    Add compiler flags to support profiling the library code with gprof.

BUILD_DOCUMENTATION (boolean)

    Look for doxygen. If it is found, use it to generate HTML
    documentation of the library source code.

BUILD_TESTING (boolean)

    Build all the regression tests for the library. This is usually only needed
    if you are planning to modify the library and want to ensure that
    you do not introduce a regression in the code.
