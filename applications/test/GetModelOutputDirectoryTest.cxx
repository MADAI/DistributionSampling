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
  std::string settingsFile( "GetModelOutputDirectoryTest.dat" );
  std::ofstream tempFile( settingsFile.c_str(),
                          std::ofstream::out | std::ofstream::trunc );
  if ( !tempFile.good() ) {
    std::cerr << "Could not open temporary file for writing.\n";
    return EXIT_FAILURE;
  }

  std::string modelDirectory( "modelDirectory" );

  tempFile << "MODEL_OUTPUT_DIRECTORY " << modelDirectory << "\n";
  tempFile.close();

  madai::RuntimeParameterFileReader settings;
  settings.ParseFile( settingsFile );

  // Try current directory
  std::string statisticsDirectory( "." );
  std::string expectedDirectory( "." );
  expectedDirectory += madai::Paths::SEPARATOR + modelDirectory;
  std::string actualDirectory =
    madai::GetModelOutputDirectory( statisticsDirectory, settings );
  if ( expectedDirectory != actualDirectory ) {
    std::cerr << "Model directory was expected to be '" << expectedDirectory
              << "' but got '" << actualDirectory << "'.\n";
    return EXIT_FAILURE;
  }

  // Try full path
  statisticsDirectory = madai::Paths::SEPARATOR + std::string( "home" )
    + madai::Paths::SEPARATOR;
  expectedDirectory = madai::Paths::SEPARATOR + std::string( "home" )
    + madai::Paths::SEPARATOR + modelDirectory;

  actualDirectory = madai::GetModelOutputDirectory( statisticsDirectory, settings );
  if ( expectedDirectory != actualDirectory ) {
    std::cerr << "Model directory was expected to be '" << expectedDirectory
              << "' but got '" << actualDirectory << "'.\n";
    return EXIT_FAILURE;
  }

  // Try relative path
  // Try full path
  statisticsDirectory = std::string( "home" ) + madai::Paths::SEPARATOR;
  expectedDirectory = std::string( "home" )
    + madai::Paths::SEPARATOR + modelDirectory;

  actualDirectory = madai::GetModelOutputDirectory( statisticsDirectory, settings );
  if ( expectedDirectory != actualDirectory ) {
    std::cerr << "Model directory was expected to be '" << expectedDirectory
              << "' but got '" << actualDirectory << "'.\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
