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
#include <cstdlib>
#include <iostream>

#include "ApplicationUtilities.h"
#include "Paths.h"
#include "RuntimeParameterFileReader.h"


int main( int, char *[] ) {
  // Write a temporary file
  std::string settingsFile( "GetExperimentalResultsFileTest.dat" );
  std::ofstream tempFile( settingsFile.c_str(),
                          std::ofstream::out | std::ofstream::trunc );
  if ( !tempFile.good() ) {
    std::cerr << "Could not open temporary file for writing.\n";
    return EXIT_FAILURE;
  }

  std::string experimentalResultsFile( "experimental_results.dat" );

  tempFile << "EXPERIMENTAL_RESULTS_FILE " << experimentalResultsFile << "\n";
  tempFile.close();

  madai::RuntimeParameterFileReader settings;
  settings.ParseFile( settingsFile );

  // Try current directory
  std::string statisticsDirectory( "." );
  std::string expectedFile( "./experimental_results.dat" );
  std::string actualFile =
    madai::GetExperimentalResultsFile( statisticsDirectory, settings );
  if ( expectedFile != actualFile ) {
    std::cerr << "Experimental results file was expected to be '"
              << expectedFile << "' but got '" << actualFile << "'.\n";
    return EXIT_FAILURE;
  }

  // Try full path
  statisticsDirectory = madai::Paths::SEPARATOR + std::string( "home" )
    + madai::Paths::SEPARATOR;
  expectedFile = madai::Paths::SEPARATOR + std::string( "home" )
    + madai::Paths::SEPARATOR + experimentalResultsFile;

  actualFile = madai::GetExperimentalResultsFile( statisticsDirectory, settings );
  if ( expectedFile != actualFile ) {
    std::cerr << "Experimental results file was expected to be '" << expectedFile
              << "' but got '" << actualFile << "'.\n";
    return EXIT_FAILURE;
  }

  // Try relative path
  // Try full path
  statisticsDirectory = std::string( "home" ) + madai::Paths::SEPARATOR;
  expectedFile = std::string( "home" )
    + madai::Paths::SEPARATOR + experimentalResultsFile;

  actualFile = madai::GetExperimentalResultsFile( statisticsDirectory, settings );
  if ( expectedFile != actualFile ) {
    std::cerr << "Experimental results file was expected to be '" << expectedFile
              << "' but got '" << actualFile << "'.\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
