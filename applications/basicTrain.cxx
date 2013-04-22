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

#include "ApplicationUtilities.h"
#include "RuntimeParameterFileReader.h"
#include "GaussianProcessEmulator.h"
#include "GaussianProcessEmulatorDirectoryReader.h"
#include "GaussianProcessEmulatorSingleFileReader.h"
#include "GaussianProcessEmulatorSingleFileWriter.h"
#include "Paths.h"

struct RuntimeOptions 
{
  madai::GaussianProcessEmulator::CovarianceFunctionType covFunct;
  int regressionOrder;
  double Nugget;
  double amplitude;
  double scale;
};

bool parseRuntimeOptions( int argc, char* argv[], struct RuntimeOptions & options)
{
  options.covFunct = madai::GaussianProcessEmulator::SQUARE_EXPONENTIAL_FUNCTION;
  options.regressionOrder = 1;
  options.Nugget = 1e-3;
  options.amplitude = 1.0;
  options.scale = 1e-2;
  
  for ( unsigned int i = 0; i < argc; i++ ) {
    std::string argString( argv[i] );
    
    if ( argString == "EMULATOR_COVARIANCE_FUNCTION" ) {
      std::string CovType( argv[i+1] );
      if ( CovType == "EMULATOR_POWER_EXPONENTIAL_FUNCTION" ) {
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
  std::string TopDirectory;
  std::string outputFile;
  std::string PCAFile;
  if (argc > 1) {
    TopDirectory = std::string(argv[1]);
    madai::EnsurePathSeparatorAtEnd( TopDirectory );
  } else {
    std::cerr << useage << '\n';
    return EXIT_FAILURE;
  }
  madai::RuntimeParameterFileReader RPFR;
  RPFR.ParseFile( TopDirectory + madai::Paths::STATISTICAL_ANALYSIS_DIRECTORY +
                  madai::Paths::SEPARATOR + "MCMC.dat" );
  char** Args = RPFR.m_Arguments;
  int NArgs = RPFR.m_NumArguments;
  struct RuntimeOptions options;
  if ( !parseRuntimeOptions( NArgs, Args, options ) ) {
    std::cerr << "Error parsing runtime options\n";
    return EXIT_FAILURE;
  }
  outputFile = TopDirectory + madai::Paths::STATISTICAL_ANALYSIS_DIRECTORY +
    madai::Paths::SEPARATOR + madai::Paths::EMULATOR_STATE_FILE;

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
