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
trainEmulator
  Tune hyperparameters for a N-D Gaussian Process Model Emulator

ACKNOWLEDGMENTS:
  This software was written in 2012-2013 by Hal Canary
  <cs.unc.edu/~hal>, based off of the MADAIEmulator program (Copyright
  2009-2012 Duke University) by C.Coleman-Smith <cec24@phy.duke.edu>
  in 2010-2012 while working for the MADAI project <http://madai.us/>.

USE:
  For details on how to use trainEmulator, consult the manpage via:
    $ nroff -man < [PATH_TO/]trainEmulator.1 | less
  or, if the manual is installed:
    $ man 1 trainEmulator
*/

#include <getopt.h>
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

using madai::Paths;


static const madai::GaussianProcessEmulator::CovarianceFunctionType DEFAULT_COVARIACE_FUNCTION
  = madai::GaussianProcessEmulator::SQUARE_EXPONENTIAL_FUNCTION;
static const int DEFAULT_REGRESSION_ORDER = 1;

struct EmulatorTrainingRuntimeOpts{
  std::string ModelOutputDirectory;
  std::string ExperimentalResultsDirectory;
  int regressionOrder;
  madai::GaussianProcessEmulator::CovarianceFunctionType covarianceFunction;
  bool quietFlag;
};

/**
 * option parsing using getoptlong.  If it fails, returns false.
 */
bool parseEmulatorTrainingRuntimeOptions(
    int argc, char** argv,
    struct EmulatorTrainingRuntimeOpts & opts)
{
  // init with default values
  opts.regressionOrder = DEFAULT_REGRESSION_ORDER;
  opts.covarianceFunction = DEFAULT_COVARIACE_FUNCTION;
  opts.quietFlag = 0;
  opts.ModelOutputDirectory = madai::Paths::DEFAULT_MODEL_OUTPUT_DIRECTORY;
  opts.ExperimentalResultsDirectory = madai::Paths::DEFAULT_EXPERIMENTAL_RESULTS_DIRECTORY;

  for ( unsigned int i = 0; i < argc; i++ ) {
    std::string argString( argv[i] );
    
    if ( argString == "MODEL_OUTPUT_DIRECTORY" ) {
      opts.ModelOutputDirectory = std::string( argv[i+1] );
      i++;
    } else if ( argString == "EXPERIMENTAL_RESULTS_DIRECTORY" ) {
      opts.ExperimentalResultsDirectory = std::string( argv[i+1] );
      i++;
    } else if ( argString == "EMULATOR_COVARIANCE_FUNCTION" ) {
      std::string CovType( argv[i+1] );
      if ( CovType == "POWER_EXPONENTIAL_FUNCTION" ) {
        opts.covarianceFunction = madai::GaussianProcessEmulator::POWER_EXPONENTIAL_FUNCTION;
      } else if ( CovType == "SQUARE_EXPONENTIAL_FUNCTION" ) {
        opts.covarianceFunction = madai::GaussianProcessEmulator::SQUARE_EXPONENTIAL_FUNCTION;
      } else if ( CovType == "MATERN_32_FUNCTION" ) {
        opts.covarianceFunction = madai::GaussianProcessEmulator::MATERN_32_FUNCTION;
      } else if ( CovType == "MATERN_52_FUNCTION" ) {
        opts.covarianceFunction = madai::GaussianProcessEmulator::MATERN_52_FUNCTION;
      } else {
        std::cerr << "Unrecognized covariance function: " << CovType << "\n";
        return false;
      }
      i++;
    } else if ( argString == "EMULATOR_REGRESSION_ORDER" ) {
      opts.regressionOrder = atoi( argv[i+1] );
      if ( opts.regressionOrder < 0 || opts.regressionOrder > 3 ) {
        std::cerr << "Error: EMULATOR_REGRESSION_ORDER given incorrect argument: \""
          << argv[i+1] << "\"\n";
          return false;
      }
      i++;
    } else if ( argString == "EMULATOR_TRAINING_QUIET_FLAG" ) {
      std::string tstring( argv[i+1] );
      if ( tstring == "0" || tstring == "false" ) {
        opts.quietFlag = false;
      } else if ( tstring == "1" || tstring == "true" ) {
        opts.quietFlag = true;
      } else {
        std::cerr << "Error: EMULATOR_TRAINING_QUIET_FLAG given incorrect argument: \""
          << tstring << "\"\n";
          return false;
      }
      i++;
    }
  }
  return true;
}

int main(int argc, char ** argv) {
  std::string StatisticsDirectory;
  if ( argc > 1 ) {
    StatisticsDirectory = std::string(argv[1]);
  } else {
    std::cerr << "Usage:\n"
              << "  trainEmulator <StatisticsDirectory>\n"
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
              << "EMULATOR_COVARIANCE_FUNCTION <value> (default: "
              << "SQUARE_EXPONENTIAL_FUNCTION)\n"
              << "EMULATOR_REGRESSION_ORDER <value> (default: 1)\n"
              << "EMULATOR_TRAINING_QUIET_FLAG <value> (default: false)\n";
    return EXIT_FAILURE;
  }
  std::string OutputFile = StatisticsDirectory;
  madai::GaussianProcessEmulator gpme;
  struct EmulatorTrainingRuntimeOpts Opts;
  if ( StatisticsDirectory == "-" ) {
    madai::GaussianProcessEmulatorSingleFileReader singleFileReader;
    singleFileReader.LoadTrainingData( &gpme, std::cin);
  } else {
    madai::EnsurePathSeparatorAtEnd( StatisticsDirectory );
    madai::RuntimeParameterFileReader RPFR;
    RPFR.ParseFile( StatisticsDirectory + 
                    madai::Paths::RUNTIME_PARAMETER_FILE );
    char** Args = RPFR.GetArguments();
    int NArgs = RPFR.GetNumberOfArguments();
    if ( !parseEmulatorTrainingRuntimeOptions( NArgs, Args, Opts ) ) {
      std::cerr << "Error parsing runtime options\n";
      return EXIT_FAILURE;
    }
    madai::GaussianProcessEmulatorDirectoryReader directoryReader;
    std::string MOD = StatisticsDirectory + Opts.ModelOutputDirectory;
    std::string ERD = StatisticsDirectory + Opts.ExperimentalResultsDirectory;
    if ( !directoryReader.LoadTrainingData(&gpme, MOD, StatisticsDirectory, ERD) ) {
      std::cerr << "Error loading training data.\n";
      return EXIT_FAILURE;
    }
    if ( !directoryReader.LoadPCA(&gpme, StatisticsDirectory) ) {
      std::cerr << "Error loading PCA data.\n";
      return EXIT_FAILURE;
    }
    OutputFile = StatisticsDirectory + madai::Paths::EMULATOR_STATE_FILE;
  }
  
  if (! gpme.Train(
          Opts.covarianceFunction,
          Opts.regressionOrder)) {
    return EXIT_FAILURE;
  }
  
  std::ofstream os( OutputFile.c_str() );

  madai::GaussianProcessEmulatorSingleFileWriter singleFileWriter;
  singleFileWriter.Write( &gpme, os );
  
  return EXIT_SUCCESS;
}
