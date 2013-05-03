#!/bin/sh
#                        EXAMPLE BUILD SCRIPT
#
#  Copyright 2013 The University of North Carolina at Chapel Hill
#  All rights reserved.
#
#  Licensed under the MADAI Software License. You may obtain a copy of
#  this license at
#
#         https://madai-public.cs.unc.edu/software/license/
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.

if [ -z "$1" ] ; then
    echo "Usage:"
    echo "  $0 INSTALL_PREFIX"
    echo "Examples:"
    echo "  $0 \"\${HOME}/local\""
    echo "  $0 \"\${HOME}/madai\""
    echo "  $0 /usr/local"
    exit 1
fi
INSTALL_PREFIX="$1"
SRC_DIR="$(cd "$(dirname "$0")" ; pwd)"
BUILD_DIR=`mktemp -d -t madai_build_XXXXXX`
cd "$BUILD_DIR"
renice 1 $$
cmake "$SRC_DIR" \
  -DCMAKE_INSTALL_PREFIX:PATH="${INSTALL_PREFIX}" \
  -DCMAKE_BUILD_TYPE:STRING=Release \
  -DBUILD_TESTING:BOOL=0 \
  -DUSE_OPENMP:BOOL=0 \
  -DUSE_GPROF:BOOL=0 \
  && make && make install
