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

#include <algorithm> // std::transform

#include "Paths.h"

namespace madai {

void EnsurePathSeparatorAtEnd( std::string & path )
{
  if ( *(path.end() - 1) != madai::Paths::SEPARATOR ) {
    path.push_back( madai::Paths::SEPARATOR );
  }
}

std::string GetModelOutputDirectory( const std::string & statisticsDirectory,
                                     const RuntimeParameterFileReader & settings )
{
  std::string modelOutputDirectory = Paths::DEFAULT_MODEL_OUTPUT_DIRECTORY;
  if ( settings.HasOption( "MODEL_OUTPUT_DIRECTORY" ) ) {
    modelOutputDirectory = settings.GetOption( "MODEL_OUTPUT_DIRECTORY" );
  }
  if ( modelOutputDirectory[0] != Paths::SEPARATOR ) {
    modelOutputDirectory.insert( 0, statisticsDirectory );
  }

  return modelOutputDirectory;
}

std::string GetExperimentalResultsDirectory( const std::string & statisticsDirectory,
                                             const RuntimeParameterFileReader & settings )
{
  std::string experimentalResultsDirectory = Paths::DEFAULT_EXPERIMENTAL_RESULTS_DIRECTORY;
  if ( settings.HasOption( "EXPERIMENTAL_RESULTS_DIRECTORY" ) ) {
    experimentalResultsDirectory = settings.GetOption( "EXPERIMENTAL_RESULTS_DIRECTORY" );
  }
  if ( experimentalResultsDirectory[0] != Paths::SEPARATOR ) {
    experimentalResultsDirectory.insert( 0, statisticsDirectory );
  }

  return experimentalResultsDirectory;
}

std::string LowerCase( char * buffer )
{
  std::string outBuffer( buffer );

  return LowerCase( outBuffer );
}

std::string LowerCase( std::string & s )
{
  std::string copy( s );
  std::transform( copy.begin(), copy.end(),
                  copy.begin(), ::tolower );

  return copy;
}


} // end namespace madai
