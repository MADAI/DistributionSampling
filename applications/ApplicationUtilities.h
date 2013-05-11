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

#ifndef madai_ApplicationUtilities_h_included
#define madai_ApplicationUtilities_h_included

#include <algorithm>
#include <string>
#include <vector>

#include "Model.h"
#include "Paths.h"
#include "RuntimeParameterFileReader.h"


namespace madai {

void EnsurePathSeparatorAtEnd( std::string & path );

std::string GetModelOutputDirectory( const std::string & statisticsDirectory,
                                     const RuntimeParameterFileReader & settings );

std::string GetExperimentalResultsFile( const std::string & statisticsDirectory,
                                        const RuntimeParameterFileReader & settings );

std::string LowerCase( char * buffer );

std::string LowerCase( std::string & s );

std::vector< std::string > SplitString( const std::string & input, char separator );

template<class S, class T>
int FindIndex(const S & v, const T & s)
{
  typename S::const_iterator it = std::find(v.begin(), v.end(), s);
  if (it == v.end())
    return -1;
  return std::distance(v.begin(), it);
}

Model::ErrorType LoadObservations(Model * model, std::istream & i);


} // end namespace madai


#endif // madai_ApplicationUtilities_h_included
