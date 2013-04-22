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

#include "ApplicationUtilities.h"
#include "GaussianProcessEmulator.h"
#include "GaussianProcessEmulatorDirectoryReader.h"
#include "GaussianProcessEmulatorSingleFileReader.h"
#include "GaussianProcessEmulatorSingleFileWriter.h"
#include "RuntimeParameterFileReader.h"
#include "Paths.h"


int main( int argc, char ** argv ) {
  std::string StatisticsDirectory;
  std::string ModelOutputDirectory = madai::Paths::MODEL_OUTPUT_DIRECTORY;
  std::string ExperimentalResultsDirectory = madai::Paths::EXPERIMENTAL_RESULTS_DIRECTORY;
  double fractionResolvingPower = 0.95;
  if ( argc > 1 ) {
    StatisticsDirectory = std::string( argv[1] );
  } else {
    std::cerr << "Usage:\n"
              << "    PCADecompose StatisticsDirectory\n"
              << "\n"
              << "StatisticsDirectory is the directory in which all statistics data\n"
              << "is stored. Contains the parameter file stat_params.dat\n"
              << "\n"
              << "Format of stat_params.dat:\n"
              << "PCA_FRACTION_RESOLVING_POWER <value>\n"
              << "MODEL_OUTPUT_DIRECTORY <value>\n"
              << "EXPERIMENTAL_RESULTS_DIRECTORY <value>\n"
              << "\n"
              << "Default values (if not specified) in order of listed:\n"
              << ".95\n"
              << "model_output\n"
              << "experimental_results\n";
    return EXIT_FAILURE;
  }
  madai::GaussianProcessEmulator gpme;
  if ( StatisticsDirectory == "-" ) { // Check to see if read from cin
    madai::GaussianProcessEmulatorSingleFileReader singleFileReader;
    singleFileReader.LoadTrainingData(&gpme, std::cin);
  } else { // Reads from directory structure
    madai::EnsurePathSeparatorAtEnd( StatisticsDirectory );
    madai::RuntimeParameterFileReader RPFR;
    RPFR.ParseFile( StatisticsDirectory + 
                    madai::Paths::RUNTIME_PARAMETER_FILE ); // Read in runtime parameters
    char** Args = RPFR.GetArguments();
    int NArgs = RPFR.GetNumberOfArguments();
    for ( unsigned int i = 0; i < NArgs; i++ ) {
      std::string argString(Args[i]);
      if ( argString == "PCA_FRACTION_RESOLVING_POWER" ) {
        fractionResolvingPower = atof( Args[i+1] );
        if ( fractionResolvingPower < 0 || fractionResolvingPower > 1 ) {
          std::cerr << "Resolving Power is out of range : "
          << fractionResolvingPower << "\n";
          return EXIT_FAILURE;
        }
        std::cout << "Using fractional resolving power = " 
        << fractionResolvingPower << "\n";
        i++;
      } else if ( argString == "MODEL_OUTPUT_DIRECTORY" ) {
        ModelOutputDirectory = std::string( Args[i+1] );
        i++;
      } else if ( argString == "EXPERIMENTAL_RESULTS_DIRECTORY" ) {
        ExperimentalResultsDirectory = std::string( Args[i+1] );
        i++;
      } else {
        // Skip other elements since I'm using a single configuration file
      }
    }
    // Read in the training data
    std::string MOD = StatisticsDirectory + ModelOutputDirectory;
    std::string ERD = StatisticsDirectory + ExperimentalResultsDirectory;
    madai::GaussianProcessEmulatorDirectoryReader directoryReader;
    if ( !directoryReader.LoadTrainingData(&gpme, MOD, StatisticsDirectory, ERD ) ) {
      std::cerr << "Error Loading Training Data.\n";
      return EXIT_FAILURE;
    }
  }
  
  std::string outputFileName = StatisticsDirectory + madai::Paths::PCA_DECOMPOSITION_FILE;

  // get the PCA decomposition
  if ( !gpme.PrincipalComponentDecompose( fractionResolvingPower ) ) {
    return EXIT_FAILURE;
  }
  std::ofstream os( outputFileName.c_str() );

  madai::GaussianProcessEmulatorSingleFileWriter singleFileWriter;
  singleFileWriter.WritePCA(&gpme,os);

  return EXIT_SUCCESS;
}
