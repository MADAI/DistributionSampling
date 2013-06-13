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
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "ApplicationUtilities.h"

#include "Gaussian2DModel.h"
#include "MetropolisHastingsSampler.h"


int main( int, char *[] ) {

  // Create file listing inactive parameters
  std::string settingsFile( "SetInactiveParametersTest.dat" );
  std::ofstream tempFile( settingsFile.c_str(),
                          std::ofstream::out | std::ofstream::trunc );
  if ( !tempFile.good() ) {
    std::cerr << "Could not open temporary file for writing.\n";
    return EXIT_FAILURE;
  }

  double expectedX = 12.0;
  double expectedY = 3.2;

  tempFile << "X " << expectedX << "\n";
  tempFile << "Y " << expectedY << "\n";
  tempFile.close();

  // Set up model and sampler
  madai::Gaussian2DModel model;
  madai::MetropolisHastingsSampler sampler;
  sampler.SetModel( &model );

  bool result = madai::SetInactiveParameters( settingsFile, sampler, false );

  if ( result != true ) {
    std::cerr << "Error when setting inactive parameters.\n";
    return EXIT_FAILURE;
  }

  // Check that parameters were deactivated
  if ( sampler.IsParameterActive( "X" ) ) {
    std::cerr << "Parameter X should not be active, but is.\n";
    return EXIT_FAILURE;
  }

  if ( sampler.IsParameterActive( "Y" ) ) {
    std::cerr << "Parameter Y should not be active, but is.\n";
    return EXIT_FAILURE;
  }

  // Check that the parameter values are as expected.
  if ( sampler.GetParameterValue( "X" ) != expectedX ) {
    std::cerr << "Expected X to have value " << expectedX << " but got "
              << sampler.GetParameterValue( "X" ) << " instead.\n";
    return EXIT_FAILURE;
  }

  if ( sampler.GetParameterValue( "Y" ) != expectedY ) {
    std::cerr << "Expected Y to have value " << expectedY << " but got "
              << sampler.GetParameterValue( "Y" ) << " instead.\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
