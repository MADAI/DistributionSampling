/*=========================================================================
 *
 *  Copyright 2011-2013 The University of North Carolina at Chapel Hill
 *  All rights reserved.
 *
 *  Licensed under the MADAI Software License. You may obtain a copy of
 *  this license at
 *
 *         https://madai-public.cs.unc.edu/software/license/
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

/**
printThetas
  Set hyperparameters for a N-D Gaussian Process Model Emulator

ACKNOWLEDGMENTS:
  This software was written in 2012-2013 by Hal Canary
  <cs.unc.edu/~hal>, based off of the MADAIEmulator program (Copyright
  2009-2012 Duke University) by C.Coleman-Smith <cec24@phy.duke.edu>
  in 2010-2012 while working for the MADAI project <http://madai.us/>.

USE:
  For details on how to use printThetas, consult the manpage via:
    $ nroff -man < [PATH_TO/]printThetas.1 | less
  or, if the manual is installed:
    $ man 1 printThetas
*/

const char useage [] =
  "Usage:\n"
  "    printThetas ModelSnapshotFile\n"
  "\n"
  "ModelSnapshotFile can be \"-\" or read from standard input.\n";

#include <iostream> // cout, cin
#include <fstream> // ifstream, ofstream

#include "GaussianProcessEmulator.h"

int main(int argc, char ** argv) {
  char const * inputFile = NULL;
  if (argc > 1) {
    inputFile = argv[1];
  } else {
    std::cerr << useage << '\n';
    return EXIT_FAILURE;
  }

  madai::GaussianProcessEmulator gpe;
  std::ostream & output = std::cout;

  std::istream * input = & std::cin;
  std::ifstream inputFileStream;

  if (0 != strcmp(inputFile, "-")) {
    inputFileStream.open(inputFile);
    if (! inputFileStream.good())
      std::cerr << "Error (1) loading file " << inputFile << '\n';
    input = & inputFileStream;
  }
  if (! gpe.Load(* input)) {
    std::cerr << "Error (2) loading file " << inputFile << '\n';
    return EXIT_FAILURE;
  }
  if (gpe.GetStatus() != madai::GaussianProcessEmulator::READY) {
    std::cerr << "Error (3) loading file " << inputFile << '\n';
    return EXIT_FAILURE;
  }
  if(! gpe.PrintThetas(output)) {
    std::cerr << "Error printing Thetas.\n";
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
