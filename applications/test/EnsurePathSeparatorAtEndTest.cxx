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
#include "Paths.h"


int main( int, char *[] ) {
  std::string noSeparatorAtEnd( "path" );
  std::string separatorAtEnd( noSeparatorAtEnd );
  separatorAtEnd += madai::Paths::SEPARATOR;

  std::string expectedString( separatorAtEnd );

  madai::EnsurePathSeparatorAtEnd( noSeparatorAtEnd );
  if ( expectedString != noSeparatorAtEnd ) {
    std::cerr << "Expected string was '" << expectedString << "' but got '"
              << noSeparatorAtEnd << "'.\n";
    return EXIT_FAILURE;
  }

  madai::EnsurePathSeparatorAtEnd( separatorAtEnd );
  if ( expectedString != separatorAtEnd ) {
    std::cerr << "Expected string was '" << expectedString << "' but got '"
              << separatorAtEnd << "'.\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
