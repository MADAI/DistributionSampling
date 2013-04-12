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
  "    basicTrain RootDirectory\n"
  "\n"
  "RootDirectory is the directory in which the folders model_output/ \n"
  "experimental_results/ and statistical_analysis/ are contained.\n"
  "\n"
  "This loads the model data and PCA information in order to train\n"
  "the emulator.\n";

#include <iostream> // cout, cin
#include <fstream> // ifstream, ofstream

#include "GaussianProcessEmulator.h"
#include "GaussianProcessEmulatorDirectoryReader.h"
#include "GaussianProcessEmulatorSingleFileReader.h"


int main(int argc, char ** argv) {
  std::string TopDirectory;
  std::string outputFile;
  std::string PCAFile;
  if (argc > 1) {
    TopDirectory = std::string(argv[1]);
  } else {
    std::cerr << useage << '\n';
    return EXIT_FAILURE;
  }
  outputFile = TopDirectory+"/statistical_analysis/EmulatorState.dat";

  madai::GaussianProcessEmulator gpme;
  if ( TopDirectory == "-" ) {
    madai::GaussianProcessEmulatorSingleFileReader singleFileReader;
    singleFileReader.Load(&gpme,std::cin);
  } else {
    madai::GaussianProcessEmulatorDirectoryReader directoryReader;
    if ( !directoryReader.LoadTrainingData(&gpme,TopDirectory) ) {
      std::cerr << "Error Loading Training Data.\n";
      return EXIT_FAILURE;
    }
    if ( !directoryReader.LoadPCA(&gpme,TopDirectory) ) {
      std::cerr << "Error Loading PCA Data.\n";
      return EXIT_FAILURE;
    }
  }

  madai::GaussianProcessEmulator::CovarianceFunctionType covarianceFunction
    = madai::GaussianProcessEmulator::SQUARE_EXPONENTIAL_FUNCTION;
  int regressionOrder = 1;
  double defaultNugget = 1e-3;
  double amplitude = 1.0;
  double scale = 1e-2;

  if (! gpme.BasicTraining(
          covarianceFunction,
          regressionOrder,
          defaultNugget,
          amplitude,
          scale))
    return EXIT_FAILURE;
  
  std::ofstream os (outputFile.c_str());
  gpme.Write(os);
  
  return EXIT_SUCCESS;
}
