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

#include "ApplicationUtilities.h"
#include "GaussianProcessEmulator.h"
#include "GaussianProcessEmulatorDirectoryReader.h"
#include "GaussianProcessEmulatorSingleFileWriter.h"
#include "RuntimeParameterFileReader.h"
#include "Paths.h"

using madai::Paths;


int main( int argc, char ** argv )
{
  if ( argc < 2 ) {
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
              << Paths::DEFAULT_EXPERIMENTAL_RESULTS_DIRECTORY << ")\n"
              << "READER_VERBOSE <value> (default: false )\n";

    return EXIT_FAILURE;
  }

  std::string statisticsDirectory = std::string( argv[1] );
  madai::EnsurePathSeparatorAtEnd( statisticsDirectory );

  // Read in runtime parameters
  madai::RuntimeParameterFileReader settings;
  std::string settingsFile = statisticsDirectory +
    madai::Paths::RUNTIME_PARAMETER_FILE;
  if ( !settings.ParseFile( settingsFile ) ) {
    std::cerr << "Could not open runtime parameter file '" << settingsFile << "'\n";
    return EXIT_FAILURE;
  }

  std::string modelOutputDirectory =
    madai::GetModelOutputDirectory( statisticsDirectory, settings );
  std::string experimentalResultsDirectory =
    madai::GetExperimentalResultsDirectory( statisticsDirectory, settings );

  // Read in the training data
  madai::GaussianProcessEmulator gpe;
  madai::GaussianProcessEmulatorDirectoryReader directoryReader;
  bool verbose = settings.GetOptionAsBool( "READER_VERBOSE", false );
  directoryReader.SetVerbose( verbose );
  if ( !directoryReader.LoadTrainingData( &gpe,
                                          modelOutputDirectory,
                                          statisticsDirectory,
                                          experimentalResultsDirectory ) ) {
    std::cerr << "Error loading training data.\n";
    return EXIT_FAILURE;
  }

  std::string outputFileName = statisticsDirectory +
    madai::Paths::PCA_DECOMPOSITION_FILE;

  // get the PCA decomposition
  if ( !gpe.PrincipalComponentDecompose() ) {
    return EXIT_FAILURE;
  }

  std::ofstream os( outputFileName.c_str() );

  madai::GaussianProcessEmulatorSingleFileWriter singleFileWriter;
  singleFileWriter.WritePCA( &gpe, os );

  return EXIT_SUCCESS;
}
