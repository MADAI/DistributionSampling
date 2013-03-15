Project: DistributionSampling
README.txt author: Cory Quammen <cquammen@cs.unc.edu>,
    Hal Canary <cs.unc.edu/~hal>
Last updated: 2013-03-10

*** BUILD INSTRUCTIONS FOR LINUX AND MAC ***

** PREREQUISITES **

-   A compiler.

-   CMake version 2.8 or greater
    (Available at http://www.cmake.org/cmake/resources/software.html
    or "yum install cmake" or "apt-get install cmake".)

-   The Boost Library (http://www.boost.org/users/download/
    or "yum install boost-devel" or "apt-get install libboost-dev")

-   The Eigen3 Library (http://eigen.tuxfamily.org/
    or "yum install eigen3-devel" or "apt-get install libeigen3-dev")

** CONFIGURING **

1). Create a build directory outside the git repository in which this
    file is located. For example, if the directory containing this
    README.txt file is located in

    /home/user/cquammen/madai/stat/DistributionSampling

    a good build directory might be

    /home/user/cquammen/madai/build/stat/DistributionSampling-build

2). Navigate to the build directory created in step 1.

3). Type ("$ " represents your shell prompt.)

    $ cmake ".../stat/DistributionSampling" \
          -DBUILD_TESTING:BOOL="1" \
          -DCMAKE_BUILD_TYPE:STRING="Debug"

    where .../stat/DistributionSampling is an absolute or relative path
    to the DistributionSampling source directory.

    Using the example directories above, this command is

    $ cmake "/home/user/cquammen/madai/stat/DistributionSampling" \
          -DBUILD_TESTING:BOOL="1" \
          -DCMAKE_BUILD_TYPE:STRING="Debug"

4). Type

    $ make

    The various source files will compile, producing a library
    libDistributionSampling.a in a directory called "lib" in the root
    level of the build directory.

5.  (optional) Type

    $ ctest

    This runs the tests defined in the project.
