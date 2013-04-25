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

#ifndef madai_ApplicationUtilities_h_included
#define madai_ApplicationUtilities_h_included

#include <string>

#include "Paths.h"
#include "RuntimeParameterFileReader.h"


namespace madai {

void EnsurePathSeparatorAtEnd( std::string & path );

std::string GetModelOutputDirectory( const std::string & statisticsDirectory,
                                     const RuntimeParameterFileReader & settings );

std::string GetExperimentalResultsDirectory( const std::string & statisticsDirectory,
                                             const RuntimeParameterFileReader & settings );

std::string LowerCase( char * buffer );

std::string LowerCase( std::string & s );

} // end namespace madai


#endif // madai_ApplicationUtilities_h_included
