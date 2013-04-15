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
PCADecompose
   Decompose the model data from a directory structure.
*/
#include <iostream>
#include <fstream>

#include "GaussianProcessEmulator.h"
#include "Paths.h"
#include "GaussianProcessEmulatorDirectoryReader.h"
#include "GaussianProcessEmulatorSingleFileReader.h"
#include "GaussianProcessEmulatorSingleFileWriter.h"


int main( int argc, char ** argv ) {
  std::string TopDirectory;
  double fractionResolvingPower = 0.95;
  if ( argc > 1 ) {
    TopDirectory = std::string( argv[1] );
    if ( argc > 2 ) {
      fractionResolvingPower = atof( argv[2] );
    }
  } else {
    std::cerr << "Usage:\n";
    std::cerr << "    PCADecompose RootDirectory [fractionResolvingPower]\n";
    std::cerr << "\n";
    std::cerr << "RootDirectory is the directory containing the directories"
              << madai::Paths::MODEL_OUTPUT_DIRECTORY << "/\n";
    std::cerr << madai::Paths::EXPERIMENTAL_RESULTS << ", and "
              << madai::Paths::STATISTICAL_ANALYSIS_DIRECTORY << "/\n";
    std::cerr << "\n";
    std::cerr << "[fractionResolvingPower is the faction of the power of the emulator\n";
    std::cerr << "at resolving the data into different components. Defaults to .95\n";
    return EXIT_FAILURE;
  }
  std::string outputFileName = TopDirectory + madai::Paths::SEPARATOR +
    madai::Paths::STATISTICAL_ANALYSIS_DIRECTORY + madai::Paths::SEPARATOR +
    madai::Paths::PCA_DECOMPOSITION_FILE;

  madai::GaussianProcessEmulator gpme;
  if ( TopDirectory == "-" ) {
    madai::GaussianProcessEmulatorSingleFileReader singleFileReader;
    singleFileReader.LoadTrainingData(&gpme, std::cin);
  } else {
    madai::GaussianProcessEmulatorDirectoryReader directoryReader;
    if ( !directoryReader.LoadTrainingData(&gpme,TopDirectory) ) {
      std::cerr << "Error Loading Training Data.\n";
      return EXIT_FAILURE;
    }
  }
  // get the PCA decomposition
  if ( !gpme.PrincipalComponentDecompose( fractionResolvingPower ) ) {
    return EXIT_FAILURE;
  }
  std::ofstream os( outputFileName.c_str() );

  madai::GaussianProcessEmulatorSingleFileWriter singleFileWriter;
  singleFileWriter.WritePCA(&gpme,os);

  return EXIT_SUCCESS;
}
