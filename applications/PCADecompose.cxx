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

#include "ApplicationUtilities.h"
#include "GaussianProcessEmulator.h"
#include "GaussianProcessEmulatorDirectoryReader.h"
#include "GaussianProcessEmulatorSingleFileWriter.h"
#include "RuntimeParameterFileReader.h"
#include "Paths.h"

using madai::Paths;


int main( int argc, char ** argv )
{
  std::string statisticsDirectory;
  if ( argc > 1 ) {
    statisticsDirectory = std::string( argv[1] );
    madai::EnsurePathSeparatorAtEnd( statisticsDirectory );
  } else {
    std::cerr << "Usage:\n"
              << "    PCADecompose <StatisticsDirectory>\n"
              << "\n"
              << "This program performs a principal components analysis on \n"
              << "experimental data. It stores the results in \n"
              << "<StatisticsDirectory>" << Paths::SEPARATOR
              << Paths::PCA_DECOMPOSITION_FILE << "\n"
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
              << Paths::DEFAULT_EXPERIMENTAL_RESULTS_DIRECTORY << ")\n";
    return EXIT_FAILURE;
  }

  madai::GaussianProcessEmulator gpme;

  // Read in runtime parameters
  madai::RuntimeParameterFileReader runtimeParameters;
  std::string runtimeParameterFile = statisticsDirectory + 
    madai::Paths::RUNTIME_PARAMETER_FILE;
  if ( !runtimeParameters.ParseFile( runtimeParameterFile ) ) {
    std::cerr << "Could not open runtime parameter file '"
              << runtimeParameterFile << "'\n";
    return EXIT_FAILURE;
  }

  std::string modelOutputDirectory =
    Paths::DEFAULT_MODEL_OUTPUT_DIRECTORY;
  std::string experimentalResultsDirectory =
    Paths::DEFAULT_EXPERIMENTAL_RESULTS_DIRECTORY;

  if ( runtimeParameters.HasOption( "MODEL_OUTPUT_DIRECTORY" ) ) {
    modelOutputDirectory =
      runtimeParameters.GetOption( "MODEL_OUTPUT_DIRECTORY" );
  }

  if ( runtimeParameters.HasOption( "EXPERIMENTAL_RESULTS_DIRECTORY" ) ) {
    experimentalResultsDirectory =
      runtimeParameters.GetOption( "EXPERIMENTAL_RESULTS_DIRECTORY" );
  }

  // Read in the training data
  std::string modelOutputPath = statisticsDirectory + modelOutputDirectory;
  std::string experimentalResultsPath = statisticsDirectory +
    experimentalResultsDirectory;
  madai::GaussianProcessEmulatorDirectoryReader directoryReader;
  if ( !directoryReader.LoadTrainingData(&gpme,
                                         modelOutputPath,
                                         statisticsDirectory,
                                         experimentalResultsPath ) ) {
    std::cerr << "Error loading training data.\n";
    return EXIT_FAILURE;
  }
  
  std::string outputFileName = statisticsDirectory +
    madai::Paths::PCA_DECOMPOSITION_FILE;

  // get the PCA decomposition
  if ( !gpme.PrincipalComponentDecompose() ) {
    return EXIT_FAILURE;
  }

  std::ofstream os( outputFileName.c_str() );

  madai::GaussianProcessEmulatorSingleFileWriter singleFileWriter;
  singleFileWriter.WritePCA(&gpme,os);

  return EXIT_SUCCESS;
}
