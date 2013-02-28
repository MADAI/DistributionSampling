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

# Change these variables:
CORAL_DIR="${HOME}/build/coral"
MADAIEMULATOR_DIR="${HOME}/build/MADAIEmulator"
TARGET_BUILD_DIR="${HOME}/build/DistributionSampling"
BUILD_TYPE="Debug"
#BUILD_TYPE="Release"

SRC_DIR="$(cd "$(dirname "$0")" ; pwd)"
mkdir -p "$TARGET_BUILD_DIR"
cd "$TARGET_BUILD_DIR"
cmake "$SRC_DIR" \
  -Dcoral_DIR:PATH="${CORAL_DIR}" \
  -DMADAIEmulator_DIR:PATH="${MADAIEMULATOR_DIR}" \
  -DBUILD_TESTING:BOOL="1" \
  -DCMAKE_BUILD_TYPE:STRING="${BUILD_TYPE}" \
  && make && echo "SUCCESS:" "$TARGET_BUILD_DIR"

