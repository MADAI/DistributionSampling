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


int main( int, char *[] ) {
  std::vector< int > inputVector;
  inputVector.push_back( 0 );
  inputVector.push_back( -1 );
  inputVector.push_back( 4 );
  inputVector.push_back( 5 );

  for ( size_t i = 0; i < inputVector.size(); ++i ) {
    if ( madai::FindIndex( inputVector, inputVector[i] ) !=
         static_cast< int >( i ) ) {
      std::cerr << "Index of element " << i << " in inputVector was not "
                << i << ".\n";
      return EXIT_FAILURE;
    }
  }

  // Check for invalid elements
  if ( madai::FindIndex( inputVector, 12 ) != -1 ) {
    std::cerr << "Finding invalid element should have returned -1 but did not.\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
