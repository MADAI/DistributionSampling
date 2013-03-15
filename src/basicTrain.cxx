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
basicTrain
  Set hyperparameters for a N-D Gaussian Process Model Emulator

ACKNOWLEDGMENTS:
  This software was written in 2012-2013 by Hal Canary
  <cs.unc.edu/~hal>, based off of the MADAIEmulator program (Copyright
  2009-2012 Duke University) by C.Coleman-Smith <cec24@phy.duke.edu>
  in 2010-2012 while working for the MADAI project <http://madai.us/>.

USE:
  For details on how to use basicTrain, consult the manpage via:
    $ nroff -man < [PATH_TO/]basicTrain.1 | less
  or, if the manual is installed:
    $ man 1 basicTrain
*/

const char useage [] =
  "Usage:\n"
  "    basicTrain InputModelFile [ModelSnapshotFile]\n"
  "\n"
  "InputModelFile can be \"-\" to read from standard input.\n"
  "\n"
  "ModelSnapshotFile can be \"-\" or left unspecified to write to\n"
  "standard output.\n";

#include <getopt.h>
#include <iostream> // cout, cin
#include <fstream> // ifstream, ofstream

#include "GaussianProcessEmulator.h"

int main(int argc, char ** argv) {
  char const * inputFile = NULL;
  char const * outputFile = "-";
  if (argc > 1) {
    inputFile = argv[1];
    if (argc > 2)
      outputFile = argv[2];
  } else {
    std::cerr << useage << '\n';
    return EXIT_FAILURE;
  }

  madai::GaussianProcessEmulator gpme;
  if (0 == strcmp(inputFile, "-")) {
    gpme.LoadTrainingData(std::cin);
  } else {
    std::ifstream is (inputFile);
    gpme.LoadTrainingData(is);
  }

  double fractionResolvingPower = 0.95;
  madai::GaussianProcessEmulator::CovarianceFunctionType covarianceFunction
    = madai::GaussianProcessEmulator::SQUARE_EXPONENTIAL_FUNCTION;
  int regressionOrder = 1;
  double defaultNugget = 1e-3;
  double amplitude = 1.0;
  double scale = 1e-2;

  if (! gpme.BasicTraining(
          fractionResolvingPower,
          covarianceFunction,
          regressionOrder,
          defaultNugget,
          amplitude,
          scale))
    return EXIT_FAILURE;
  if (0 == strcmp(outputFile, "-"))
    gpme.Write(std::cout);
  else {
    std::ofstream os (outputFile);
    gpme.Write(os);
  }
  return EXIT_SUCCESS;
}
