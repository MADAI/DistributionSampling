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
  std::string testString( "-a-bc-def-ghij--k-" );

  std::vector< std::string > substrings = madai::SplitString( testString, '-' );
  for ( size_t i = 0; i < substrings.size(); ++i ) {
    std::cout << substrings[i] << "\n";
  }

  return EXIT_SUCCESS;
}
