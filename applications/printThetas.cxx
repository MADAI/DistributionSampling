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
printThetas
  Set hyperparameters for a N-D Gaussian Process Model Emulator

ACKNOWLEDGMENTS:
  This software was written in 2012-2013 by Hal Canary
  <cs.unc.edu/~hal>, based off of the MADAIEmulator program (Copyright
  2009-2012 Duke University) by C.Coleman-Smith <cec24@phy.duke.edu>
  in 2010-2012 while working for the MADAI project <http://madai.us/>.

USE:
  For details on how to use printThetas, consult the manpage via:
    $ nroff -man < [PATH_TO/]printThetas.1 | less
  or, if the manual is installed:
    $ man 1 printThetas
*/

const char useage [] =
  "Usage:\n"
  "    printThetas StatisticsDirectory\n"
  "\n"
  "StatisticsDirectory is the directory in which all statistics data will\n"
  "be stored. Contains the parameter file stat_params.dat\n"
  "\n"
  "Format of stat_params.dat\n"
  "MODEL_OUTPUT_DIRECTORY <value>\n"
  "EXPERIMENTAL_RESULTS_DIRECTORY <value>\n";

#include <iostream> // cout, cin
#include <fstream> // ifstream, ofstream

#include "ApplicationUtilities.h"
#include "GaussianProcessEmulator.h"
#include "GaussianProcessEmulatorDirectoryReader.h"
#include "GaussianProcessEmulatorSingleFileReader.h"
#include "GaussianProcessEmulatorSingleFileWriter.h"
#include "RuntimeParameterFileReader.h"
#include "Paths.h"


int main(int argc, char ** argv) {
  std::string StatisticsDirectory;
  std::string ModelOutputDirectory = madai::Paths::DEFAULT_MODEL_OUTPUT_DIRECTORY;
  std::string ExperimentalResultsDirectory = 
    madai::Paths::DEFAULT_EXPERIMENTAL_RESULTS_DIRECTORY;
  if (argc > 1) {
    StatisticsDirectory = std::string( argv[1] );
  } else {
    std::cerr << useage << '\n';
    return EXIT_FAILURE;
  }
  madai::GaussianProcessEmulator gpe;
  std::ostream & output = std::cout;
  
  if ( StatisticsDirectory == "-" ) {
    madai::GaussianProcessEmulatorSingleFileReader singleFileReader;
    if ( !singleFileReader.Load(&gpe, std::cin) ) {
      std::cerr << "Error (2) loading emulator from cin\n";
      return EXIT_FAILURE;
    }
  } else {
    madai::EnsurePathSeparatorAtEnd( StatisticsDirectory );
    madai::RuntimeParameterFileReader RPFR;
    RPFR.ParseFile( StatisticsDirectory + madai::Paths::RUNTIME_PARAMETER_FILE );
    char** Args = RPFR.GetArguments();
    int NArgs = RPFR.GetNumberOfArguments();
    for ( int i = 0; i < NArgs; i++ ) {
      std::string argString( Args[i] );
      
      if ( argString == "MODEL_OUTPUT_DIRECTORY" ) {
        ModelOutputDirectory = StatisticsDirectory + std::string( Args[i+1] );
        i++;
      } else if ( argString == "EXPERIMENTAL_RESULTS_DIRECTORY" ) {
        ExperimentalResultsDirectory = StatisticsDirectory + std::string( Args[i+1] );
        i++;
      }
    }
    madai::GaussianProcessEmulatorDirectoryReader directoryReader;
    if ( !directoryReader.LoadTrainingData(&gpe, ModelOutputDirectory, StatisticsDirectory,
                                           ExperimentalResultsDirectory) ) {
      std::cerr << "Error loading training data.\n";
      return EXIT_FAILURE;
    }
    if ( !directoryReader.LoadPCA(&gpe, StatisticsDirectory) ) {
      std::cerr << "Error loading PCA data.\n";
      return EXIT_FAILURE;
    }
    if ( !directoryReader.LoadEmulator(&gpe, StatisticsDirectory) ) {
      std::cerr << "Error loading emulator data.\n";
      return EXIT_FAILURE;
    }
  }
  
  if (gpe.GetStatus() != madai::GaussianProcessEmulator::READY) {
    std::cerr << "Error (3) incomplete load from " << StatisticsDirectory << '\n';
    return EXIT_FAILURE;
  }
  madai::GaussianProcessEmulatorSingleFileWriter singleFileWriter;
  if(! singleFileWriter.PrintThetas(&gpe,output)) {
    std::cerr << "Error printing Thetas.\n";
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
