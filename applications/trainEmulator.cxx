/*=========================================================================
 *
 *  Copyright 2011-2013 The University of North Carolina at Chapel Hill
 *  All rights reserved.
 *
 *  Licensed under the MADAI Software License. You may obtain a copy of
 *  this license at
 *
 *         https://madai-public.cs.unc.edu/visualization/software-license/
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

/**
ACKNOWLEDGMENTS:
  This software was written in 2012-2013 by Hal Canary
  <cs.unc.edu/~hal>, based off of the MADAIEmulator program (Copyright
  2009-2012 Duke University) by C.Coleman-Smith <cec24@phy.duke.edu>
  in 2010-2012 while working for the MADAI project <http://madai.us/>.
*/

#include <iostream> // cout, cin
#include <fstream> // ifstream, ofstream
#include <cstring> // strcmp, strlen

#include "ApplicationUtilities.h"
#include "GaussianProcessEmulator.h"
#include "GaussianProcessEmulatorDirectoryReader.h"
#include "GaussianProcessEmulatorSingleFileReader.h"
#include "GaussianProcessEmulatorSingleFileWriter.h"
#include "RuntimeParameterFileReader.h"
#include "Paths.h"
#include "Defaults.h"

using madai::Paths;

int main(int argc, char ** argv) {

  if ( argc < 2 ) {
    std::cerr
      << "Usage:\n"
      << "    trainEmulator <StatisticsDirectory>\n"
      << "\n"
      << "This loads the model data and PCA information computed with\n"
      << "PCADecompose and performs a refined training of the emulator.\n"
      << "It stores the results in <StatisticsDirectory>" << Paths::SEPARATOR
      << Paths::EMULATOR_STATE_FILE << "\n"
      << "\n"
      << "<StatisticsDirectory> is the directory in which all \n"
      << "statistics data are stored. It contains the parameter file "
      << Paths::RUNTIME_PARAMETER_FILE << "\n"
      << "\n"
      << "Format of entries in " << Paths::RUNTIME_PARAMETER_FILE
      << ":\n\n"
      << "MODEL_OUTPUT_DIRECTORY <value> (default: "
      << madai::Defaults::MODEL_OUTPUT_DIRECTORY << ")\n"
      << "EXPERIMENTAL_RESULTS_FILE <value> (default: "
      << madai::Defaults::EXPERIMENTAL_RESULTS_FILE << ")\n"
      << "PCA_FRACTION_RESOLVING_POWER <value> (default: 0.95)\n"
      << "EMULATOR_COVARIANCE_FUNCTION <value> (default: "
      << madai::Defaults::EMULATOR_COVARIANCE_FUNCTION << ")\n"
      << "EMULATOR_REGRESSION_ORDER <value> (default: "
      << madai::Defaults::EMULATOR_REGRESSION_ORDER << ")\n"
      << "READER_VERBOSE <value> (default: false)\n"
      << "VERBOSE <value> (default: false)\n";

    return EXIT_FAILURE;
  }
  std::string statisticsDirectory( argv[1] );
  madai::EnsurePathSeparatorAtEnd( statisticsDirectory );

  madai::RuntimeParameterFileReader settings;
  std::string settingsFile = statisticsDirectory + madai::Paths::RUNTIME_PARAMETER_FILE;
  if ( !settings.ParseFile( settingsFile ) ) {
    std::cerr << "Could not open runtime parameter file '" << settingsFile << "'\n";
    return EXIT_FAILURE;
  }

  std::string modelOutputDirectory =
    madai::GetModelOutputDirectory( statisticsDirectory, settings );
  std::string experimentalResultsFile =
    madai::GetExperimentalResultsFile( statisticsDirectory, settings );

  std::string covarianceFunctionString = madai::Defaults::EMULATOR_COVARIANCE_FUNCTION;
  if ( settings.HasOption( "EMULATOR_COVARIANCE_FUNCTION" ) ) {
    covarianceFunctionString = settings.GetOption( "EMULATOR_COVARIANCE_FUNCTION" );
  }
  madai::GaussianProcessEmulator::CovarianceFunctionType emulatorCovarianceFunction;
  if ( covarianceFunctionString == "POWER_EXPONENTIAL_FUNCTION" ) {
    emulatorCovarianceFunction = madai::GaussianProcessEmulator::POWER_EXPONENTIAL_FUNCTION;
  } else if ( covarianceFunctionString == "SQUARE_EXPONENTIAL_FUNCTION" ) {
    emulatorCovarianceFunction = madai::GaussianProcessEmulator::SQUARE_EXPONENTIAL_FUNCTION;
  } else if ( covarianceFunctionString == "MATERN_32_FUNCTION" ) {
    emulatorCovarianceFunction = madai::GaussianProcessEmulator::MATERN_32_FUNCTION;
  } else if ( covarianceFunctionString == "MATERN_52_FUNCTION" ) {
    emulatorCovarianceFunction = madai::GaussianProcessEmulator::MATERN_52_FUNCTION;
  } else {
    std::cerr << "Unrecognized covariance function '" << covarianceFunctionString << "'\n";
    return EXIT_FAILURE;
  }

  int emulatorRegressionOrder = madai::Defaults::EMULATOR_REGRESSION_ORDER;
  if ( settings.HasOption( "EMULATOR_REGRESSION_ORDER" ) ) {
    emulatorRegressionOrder = atoi( settings.GetOption( "EMULATOR_REGRESSION_ORDER" ).c_str() );
  }

  madai::GaussianProcessEmulator gpe;
  madai::GaussianProcessEmulatorDirectoryReader directoryReader;
  bool verbose = settings.GetOptionAsBool( "READER_VERBOSE", false );
  directoryReader.SetVerbose( verbose );

  if ( !directoryReader.LoadTrainingData( &gpe,
                                          modelOutputDirectory,
                                          statisticsDirectory,
                                          experimentalResultsFile ) ) {
    std::cerr << "Error loading training data.\n";
    return EXIT_FAILURE;
  }

  if ( !directoryReader.LoadPCA( &gpe, statisticsDirectory ) ) {
    std::cerr << "Error loading PCA data.\n";
    return EXIT_FAILURE;
  }
  std::string outputFileName = statisticsDirectory + madai::Paths::EMULATOR_STATE_FILE;

  if (! gpe.Train( emulatorCovarianceFunction,
                   emulatorRegressionOrder ) ) {
    std::cerr << "Error while training emulator.\n";
    return EXIT_FAILURE;
  }

  std::ofstream os( outputFileName.c_str() );

  if ( !os.good() ) {
    std::cerr << "Could not open emulator state file '" << outputFileName
              << "' for writing.\n";
    return EXIT_FAILURE;
  }

  madai::GaussianProcessEmulatorSingleFileWriter singleFileWriter;
  singleFileWriter.Write( &gpe, os );

  if ( settings.GetOptionAsBool( "VERBOSE", madai::Defaults::VERBOSE ) ) {
    std::cout << "Emulator training succeeded.\n";
    std::cout << "Wrote emulator state file '" << outputFileName << "'.\n";
  }

  return EXIT_SUCCESS;
}
