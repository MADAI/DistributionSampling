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

#include "ApplicationUtilities.h"
#include "RuntimeParameterFileReader.h"
#include "GaussianProcessEmulator.h"
#include "GaussianProcessEmulatorDirectoryReader.h"
#include "GaussianProcessEmulatorSingleFileWriter.h"
#include "Paths.h"

using madai::Paths;

static const std::string DEFAULT_EMULATOR_COVARIANCE_FUNCTION( "SQUARE_EXPONENTIAL_FUNCTION" );
static const int         DEFAULT_EMULATOR_REGRESSION_ORDER = 1;
static const double      DEFAULT_EMULATOR_NUGGET           = 1.0e-3;
static const double      DEFAULT_EMULATOR_AMPLITUDE        = 1.0;
static const double      DEFAULT_EMULATOR_SCALE            = 1.0e-2;


int main(int argc, char ** argv) {
  if (argc < 2) {
    std::cerr << "Usage:\n"
              << "    basicTrain <StatisticsDirectory>\n"
              << "\n"
              << "This loads the model data and PCA information computed with \n"
              << "PCADecompose and trains the emulator. It stores the results \n"
              << "in <StatisticsDirectory>" << Paths::SEPARATOR
              << Paths::EMULATOR_STATE_FILE << "\n"
              << "\n"
              << "<StatisticsDirectory> is the directory in which all \n"
              << "statistics data are stored. It contains the parameter file "
              << Paths::RUNTIME_PARAMETER_FILE << "\n"
              << "\n"
              << "Format of entries in " << Paths::RUNTIME_PARAMETER_FILE
              << ":\n\n"
              << "MODEL_OUTPUT_DIRECTORY <value> (default: "
              << Paths::DEFAULT_MODEL_OUTPUT_DIRECTORY << ")\n"
              << "EXPERIMENTAL_RESULTS_DIRECTORY <value> (default: "
              << Paths::DEFAULT_EXPERIMENTAL_RESULTS_DIRECTORY << ")\n"
              << "PCA_FRACTION_RESOLVING_POWER <value> (default: 0.95)\n"
              << "EMULATOR_COVARIANCE_FUNCTION <value> (default: "
              << DEFAULT_EMULATOR_COVARIANCE_FUNCTION << ")\n"
              << "EMULATOR_REGRESSION_ORDER <value> (default: "
              << DEFAULT_EMULATOR_REGRESSION_ORDER << ")\n"
              << "EMULATOR_NUGGET <value> (default: "
              << DEFAULT_EMULATOR_NUGGET << ")\n"
              << "EMULATOR_AMPLITUDE <value> (default: "
              << DEFAULT_EMULATOR_AMPLITUDE << ")\n"
              << "EMULATOR_SCALE <value> (default: "
              << DEFAULT_EMULATOR_SCALE << ")\n"
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
  std::string experimentalResultsDirectory =
    madai::GetExperimentalResultsDirectory( statisticsDirectory, settings );

  std::string covarianceFunctionString = DEFAULT_EMULATOR_COVARIANCE_FUNCTION;
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

  int emulatorRegressionOrder = DEFAULT_EMULATOR_REGRESSION_ORDER;
  if ( settings.HasOption( "EMULATOR_REGRESSION_ORDER" ) ) {
    emulatorRegressionOrder = atoi( settings.GetOption( "EMULATOR_REGRESSION_ORDER" ).c_str() );
  }

  double emulatorNugget = DEFAULT_EMULATOR_NUGGET;
  if ( settings.HasOption( "EMULATOR_NUGGET" ) ) {
    emulatorNugget = atof( settings.GetOption( "EMULATOR_NUGGET" ).c_str() );
  }

  double emulatorAmplitude = DEFAULT_EMULATOR_AMPLITUDE;
  if ( settings.HasOption( "EMULATOR_AMPLITUDE" ) ) {
    emulatorAmplitude = atof( settings.GetOption( "EMULATOR_AMPLITUDE" ).c_str() );
  }

  double emulatorScale = DEFAULT_EMULATOR_SCALE;
  if ( settings.HasOption( "EMULATOR_SCALE" ) ) {
    emulatorScale = atof( settings.GetOption( "EMULATOR_SCALE" ).c_str() );
  }

  madai::GaussianProcessEmulator gpm;
  madai::GaussianProcessEmulatorDirectoryReader directoryReader;
  bool verbose = settings.GetOptionAsBool( "READER_VERBOSE", false );
  directoryReader.SetVerbose( verbose );

  if ( !directoryReader.LoadTrainingData( &gpm,
                                          modelOutputDirectory,
                                          statisticsDirectory,
                                          experimentalResultsDirectory ) ) {
    std::cerr << "Error loading training data.\n";
    return EXIT_FAILURE;
  }
  if ( !directoryReader.LoadPCA( &gpm, statisticsDirectory ) ) {
    std::cerr << "Error loading PCA Data.\n";
    return EXIT_FAILURE;
  }

  if (! gpm.BasicTraining( emulatorCovarianceFunction,
                           emulatorRegressionOrder,
                           emulatorNugget,
                           emulatorAmplitude,
                           emulatorScale ) ) {
    std::cerr << "Error while performing basic training\n";
    return EXIT_FAILURE;
  }

  std::string outputFileName = statisticsDirectory + madai::Paths::EMULATOR_STATE_FILE;
  std::ofstream os( outputFileName.c_str() );

  if ( !os.good() ) {
    std::cerr << "Could not open emulator state file '" << outputFileName
              << "' for writing.\n";
    return EXIT_FAILURE;
  }

  madai::GaussianProcessEmulatorSingleFileWriter singleFileWriter;
  singleFileWriter.Write( &gpm, os );

  if ( settings.GetOptionAsBool( "VERBOSE", false ) ) {
    std::cout << "Emulator training succeeded.\n";
    std::cout << "Wrote emulator state file '" << outputFileName << "'.\n";
  }

  return EXIT_SUCCESS;
}
