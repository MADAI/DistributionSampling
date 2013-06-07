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
      << "    " << argv[0] << " <StatisticsDirectory>\n"
      << "\n"
      << "This loads the model data and PCA information computed with\n"
      << "madai_pca_decompose and performs a refined training of the emulator.\n"
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
      << "EMULATOR_TRAINING_RIGOR <value> (default: "
      << madai::Defaults::EMULATOR_TRAINING_RIGOR << ")\n"
      << "EMULATOR_COVARIANCE_FUNCTION <value> (default: "
      << madai::Defaults::EMULATOR_COVARIANCE_FUNCTION << ")\n"
      << "EMULATOR_REGRESSION_ORDER <value> (default: "
      << madai::Defaults::EMULATOR_REGRESSION_ORDER << ")\n"
      << "EMULATOR_NUGGET <value> (default: "
      << madai::Defaults::EMULATOR_NUGGET << ")\n"
      << "EMULATOR_AMPLITUDE <value> (default: "
      << madai::Defaults::EMULATOR_AMPLITUDE << ")\n"
      << "EMULATOR_SCALE <value> (default: "
      << madai::Defaults::EMULATOR_SCALE << ")\n"
      << "READER_VERBOSE <value> (default: "
      << madai::Defaults::READER_VERBOSE << ")\n"
      << "VERBOSE <value> (default: "
      << madai::Defaults::VERBOSE << ")\n";

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

  std::string covarianceFunctionString = settings.GetOption(
      "EMULATOR_COVARIANCE_FUNCTION",
      madai::Defaults::EMULATOR_COVARIANCE_FUNCTION);

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

  int emulatorRegressionOrder = settings.GetOptionAsInt(
    "EMULATOR_REGRESSION_ORDER", madai::Defaults::EMULATOR_REGRESSION_ORDER);

  double emulatorNugget = settings.GetOptionAsDouble(
    "EMULATOR_NUGGET", madai::Defaults::EMULATOR_NUGGET);

  double emulatorAmplitude = settings.GetOptionAsDouble(
    "EMULATOR_AMPLITUDE", madai::Defaults::EMULATOR_AMPLITUDE);

  double emulatorScale = settings.GetOptionAsDouble(
    "EMULATOR_SCALE", madai::Defaults::EMULATOR_SCALE);

  std::string emulatorTrainingRigor = settings.GetOption(
    "EMULATOR_TRAINING_RIGOR",  madai::Defaults::EMULATOR_TRAINING_RIGOR );
  std::transform( emulatorTrainingRigor.begin(), emulatorTrainingRigor.end(),
                  emulatorTrainingRigor.begin(), ::tolower );

  bool readerVerbose = settings.GetOptionAsBool(
      "READER_VERBOSE", madai::Defaults::READER_VERBOSE );

  madai::GaussianProcessEmulator gpe;
  madai::GaussianProcessEmulatorDirectoryReader directoryReader;
  directoryReader.SetVerbose( readerVerbose );

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

  // Switch between full and basic training
  if ( emulatorTrainingRigor == "full" ) {
    if (! gpe.Train( emulatorCovarianceFunction,
                     emulatorRegressionOrder ) ) {
      std::cerr << "Error while performing full emulator training.\n";
      return EXIT_FAILURE;
    }
  } else { /* Basic training */
    if (! gpe.BasicTraining( emulatorCovarianceFunction,
                             emulatorRegressionOrder,
                             emulatorNugget,
                             emulatorAmplitude,
                             emulatorScale ) ) {
      std::cerr << "Error while performing basic emulator training\n";
      return EXIT_FAILURE;
    }
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
