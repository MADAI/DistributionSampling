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
  std::string TopDirectory;
  double fractionResolvingPower = 0.95;
  if ( argc > 1 ) {
    TopDirectory = std::string( argv[1] );
    madai::EnsurePathSeparatorAtEnd( TopDirectory );
  } else {
    std::cerr << "Usage:\n";
    std::cerr << "    PCADecompose RootDirectory [fractionResolvingPower]\n";
    std::cerr << "\n";
    std::cerr << "RootDirectory is the directory containing the directories"
              << madai::Paths::MODEL_OUTPUT_DIRECTORY << "/\n";
    std::cerr << madai::Paths::EXPERIMENTAL_RESULTS << ", and "
              << madai::Paths::STATISTICAL_ANALYSIS_DIRECTORY << "/\n";
    return EXIT_FAILURE;
  }
  madai::RuntimeParameterFileReader RPFR;
  RPFR.ParseFile( TopDirectory + madai::Paths::STATISTICAL_ANALYSIS_DIRECTORY +
                  madai::Paths::SEPARATOR + "MCMC.dat" );
  char** Args = RPFR.m_Arguments;
  int NArgs = RPFR.m_NumArguments;
  for ( unsigned int i = 0; i < NArgs; i++ ) {
    std::string argString(Args[i]);
    if ( argString == "PCA_FRACTION_RESOLVING_POWER" ) {
      fractionResolvingPower = atof( Args[i+1] );
      if ( fractionResolvingPower < 0 || fractionResolvingPower > 1 ) {
        std::cerr << "Resolving Power is out of range : "
                  << fractionResolvingPower << "\n";
        return EXIT_FAILURE;
      }
      std::cerr << "Using fractional resolving power = " 
                << fractionResolvingPower << "\n";
    } else {
      // Skip other elements since I'm using a single configuration file
    }
  }
  
  std::string outputFileName = TopDirectory + madai::Paths::STATISTICAL_ANALYSIS_DIRECTORY +
    madai::Paths::SEPARATOR + madai::Paths::PCA_DECOMPOSITION_FILE;

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
