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
#include <sstream>
#include <string>

#include "ApplicationUtilities.h"

#include "Gaussian2DModel.h"


int main( int, char *[] ) {

  // Create a string with some observations for the Gaussian 2D model
  std::string observations( "X 22.0 3.2\nY 13.2 1.7\n" );

  // Create a stream from the string
  std::istringstream stream( observations );

  // Create a model
  madai::Gaussian2DModel model;

  // This should pass
  madai::Model::ErrorType error = madai::LoadObservations( &model, stream );
  if ( error != madai::Model::NO_ERROR ) {
    std::cerr << "Error encountered in LoadObservations().\n";
    return EXIT_FAILURE;
  }

  // This should fail
  error = madai::LoadObservations( &model, stream );
  if ( error == madai::Model::NO_ERROR ) {
    std::cerr << "An error should have been reported, but was not.\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
