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

#include "ApplicationUtilities.h"


int main( int, char *[] ) {
  std::string testString( "ThIs iS a TesT." );
  std::string expectedString( "this is a test." );

  std::string actualString = madai::LowerCase( testString );
  if ( actualString != expectedString ) {
    std::cerr << "Expected LowerCase( std::string & s ) to return '"
              << expectedString << "' but got '" << actualString << "'.\n";
    return EXIT_FAILURE;
  }

  actualString = madai::LowerCase( testString.c_str() );
  if ( actualString != expectedString ) {
    std::cerr << "Expected LowerCase( std::string & s ) to return '"
              << expectedString << "' but got '" << actualString << "'.\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
