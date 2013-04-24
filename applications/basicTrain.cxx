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

#include <iostream> // cout, cin
#include <fstream> // ifstream, ofstream

#include "ApplicationUtilities.h"
#include "RuntimeParameterFileReader.h"
#include "GaussianProcessEmulator.h"
#include "GaussianProcessEmulatorDirectoryReader.h"
#include "GaussianProcessEmulatorSingleFileReader.h"
#include "GaussianProcessEmulatorSingleFileWriter.h"
#include "Paths.h"

using madai::Paths;


struct RuntimeOptions 
{
  std::string ModelOutputDirectory;
  std::string ExperimentalResultsDirectory;
  madai::GaussianProcessEmulator::CovarianceFunctionType covFunct;
  int regressionOrder;
  double Nugget;
  double amplitude;
  double scale;
};

bool parseRuntimeOptions( int argc, char* argv[], struct RuntimeOptions & options)
{
  options.covFunct = madai::GaussianProcessEmulator::SQUARE_EXPONENTIAL_FUNCTION;
  options.ModelOutputDirectory = madai::Paths::DEFAULT_MODEL_OUTPUT_DIRECTORY;
  options.ExperimentalResultsDirectory = madai::Paths::DEFAULT_EXPERIMENTAL_RESULTS_DIRECTORY;
  options.regressionOrder = 1;
  options.Nugget = 1e-3;
  options.amplitude = 1.0;
  options.scale = 1e-2;
  
  for ( unsigned int i = 0; i < argc; i++ ) {
    std::string argString( argv[i] );
    
    if ( argString == "EMULATOR_COVARIANCE_FUNCTION" ) {
      std::string CovType( argv[i+1] );
      if ( CovType == "POWER_EXPONENTIAL_FUNCTION" ) {
        options.covFunct = madai::GaussianProcessEmulator::POWER_EXPONENTIAL_FUNCTION;
      } else if ( CovType == "SQUARE_EXPONENTIAL_FUNCTION" ) {
        options.covFunct = madai::GaussianProcessEmulator::SQUARE_EXPONENTIAL_FUNCTION;
      } else if ( CovType == "MATERN_32_FUNCTION" ) {
        options.covFunct = madai::GaussianProcessEmulator::MATERN_32_FUNCTION;
      } else if ( CovType == "MATERN_52_FUNCTION" ) {
        options.covFunct = madai::GaussianProcessEmulator::MATERN_52_FUNCTION;
      } else {
        std::cerr << "Unrecognized  covariance function: " << CovType << "\n";
        return false;
      }
      i++;
    } else if ( argString == "MODEL_OUTPUT_DIRECTORY" ) {
      options.ModelOutputDirectory = std::string( argv[i+1] );
      i++;
    } else if ( argString == "EXPERIMENTAL_RESULTS_DIRECTORY" ) {
      options.ExperimentalResultsDirectory = std::string( argv[i+1] );
      i++;
    } else if ( argString == "EMULATOR_REGRESSION_ORDER" ) {
      options.regressionOrder = atoi(argv[i+1]);
      std::cout << "Using regression order = " << options.regressionOrder << "\n";
      i++;
    } else if ( argString == "EMULATOR_NUGGET" ) {
      options.Nugget = atof(argv[i+1]);
      std::cout << "Using nugget = " << options.Nugget << "\n";
      i++;
    } else if ( argString == "EMULATOR_AMPLITUDE" ) {
      options.amplitude = atof(argv[i+1]);
      std::cout << "Using amplitude = " << options.amplitude << "\n";
      i++;
    } else if ( argString == "EMULATOR_SCALE" ) {
      options.scale = atof(argv[i+1]);
      std::cout << "Using scale = " << options.scale << "\n";
      i++;
    }
  }
  return true;
}


int main(int argc, char ** argv) {
  std::string StatisticsDirectory;
  std::string ModelOutputDirectory;
  std::string outputFile;
  std::string PCAFile;
  if (argc > 1) {
    StatisticsDirectory = std::string(argv[1]);
  } else {
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
              << "EMULATOR_COVARIANCE_FUNCTION <value> (default: SQUARE_EXPONENTIAL_FUNCTION)\n"
              << "EMULATOR_REGRESSION_ORDER <value> (default: 1)\n"
              << "EMULATOR_NUGGET <value> (default: 1e-3)\n"
              << "EMULATOR_AMPLITUDE <value> (default: 1.0)\n"
              << "EMULATOR_SCALE <value> (default: 1e-2)\n";

    return EXIT_FAILURE;
  }
  madai::GaussianProcessEmulator gpme;
  struct RuntimeOptions options;
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
    if ( !parseRuntimeOptions( NArgs, Args, options ) ) {
      std::cerr << "Error parsing runtime options\n";
      return EXIT_FAILURE;
    }
    std::string MOD = StatisticsDirectory+options.ModelOutputDirectory;
    std::string ERD = StatisticsDirectory+options.ExperimentalResultsDirectory;
    madai::GaussianProcessEmulatorDirectoryReader directoryReader;
    if ( !directoryReader.LoadTrainingData(&gpme, MOD, StatisticsDirectory, ERD) ) {
      std::cerr << "Error Loading Training Data.\n";
      return EXIT_FAILURE;
    }
    if ( !directoryReader.LoadPCA(&gpme, StatisticsDirectory) ) {
      std::cerr << "Error Loading PCA Data.\n";
      return EXIT_FAILURE;
    }
  }
  
  outputFile = StatisticsDirectory + madai::Paths::EMULATOR_STATE_FILE;

  if (! gpme.BasicTraining(
          options.covFunct,
          options.regressionOrder,
          options.Nugget,
          options.amplitude,
          options.scale))
    return EXIT_FAILURE;
  
  std::ofstream os (outputFile.c_str());

  madai::GaussianProcessEmulatorSingleFileWriter singleFileWriter;
  singleFileWriter.Write(&gpme,os);
  
  return EXIT_SUCCESS;
}
