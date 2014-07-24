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

namespace madai {

class Sampler;
class RuntimeParameterFileReader;

/**
 * Ensures that a path separator appropriate for the system ('/' for
 * Mac OS X and Linux, '\' for Windows) is at the end of the path
 * passed in as the argument. Modifies the input argument. */
void EnsurePathSeparatorAtEnd( std::string & path );

/**
 * Get the model output directory specified by the settings.dat file
 * in the statisticsDirectory. */
std::string GetModelOutputDirectory( const std::string & statisticsDirectory,
                                     const RuntimeParameterFileReader & settings );

/**
 * Get the experimental results file specified by the settings.dat
 * file in the statisticsDirectory. */
std::string GetExperimentalResultsFile( const std::string & statisticsDirectory,
                                        const RuntimeParameterFileReader & settings );

/**
 * Get the inactive parameter file specified by the settings.dat file
 * in the statisticsDirectory. If there is no setting
 * SAMPLER_INACTIVE_PARAMETER_FILE, then an empty string is returned. */
std::string GetInactiveParametersFile( const std::string & statisticsDirectory,
                                       const RuntimeParameterFileReader & settings );

/**
 * Get the posterior analysis directory specified by the settings.dat
 * file in the statisticsDirectory. If there is no setting
 * POSTERIOR_ANALYSIS_DIRECTORY, then an empty string is returned. */
std::string GetPosteriorAnalysisDirectory( const std::string & statisticsDirectory,
                                           const RuntimeParameterFileReader & settings );

/**
 * Opens the trace file and determines whether it has been gzipped or
 * not. This is done by checking if the first character is '"' which
 * indicates that it is not compressed. */
bool IsTraceCompressed( const std::string & traceFile );

/**
 * Returns a string where all the characters in the input parameter
 * are lowercase. */
std::string LowerCase( const char * buffer );

/**
 * Returns a string where all the characters in the input parameter
 * are lowercase. */
std::string LowerCase( const std::string & s );

/**
 * Splits the input string by the separator character. The result is a
 * vector of substrings from the original string that were separated
 * by the the separator character. Note that if the input starts with
 * a separator character, the first element in the output vector will
 * be an empty string. Similarly, if the input ends with a separator
 * character, the last element will be an empty string. Finally, if
 * there are two subsequent separators in the input, an empty string
 * will be returned as the string between those two separators. */
std::vector< std::string > SplitString( const std::string & input, char separator );

/**
 * Reads in a line from an istream and returns the tokens
 * (whitespace-separated) in a vector of strings. Ignores line content
 * from the comment character '#' and beyond. */
std::vector< std::string > ReadLineAsTokens( std::istream & is,
                                             std::string & line );

/**
 * Given a vector of type T and an argument of type T, returns the
 * index of the element in the vector that equals the argument
 * according to the == operator. If the element is not found in the
 * index, returns -1. */
template<class S, class T>
int FindIndex(const S & v, const T & s)
{
  typename S::const_iterator it = std::find(v.begin(), v.end(), s);
  if (it == v.end())
    return -1;
  return static_cast< int >( std::distance(v.begin(), it) );
}

/**
 * Given a Model and an input stream, loads observations assumed to be
 * in the input stream to defined in the stream and sets them in the
 * Model. Returns and error code based on the success of the
 * operation. */
Model::ErrorType LoadObservations(Model * model, std::istream & i);

/**
  The file located at the path stored in inactiveParametersFile has
  the same format as any option file: it can have comments preceded by
  #, which will be ignored.

  If it has a line with the format:
      PARAMETER_NAME PARAMETER_VALUE
  then that parameter will be set to that value and deactivated.

  Return true if everything works correctly.
  */
bool SetInactiveParameters(
    const std::string & inactiveParametersFile,
    madai::Sampler & sampler,
    bool verbose);


} // end namespace madai


#endif // madai_ApplicationUtilities_h_included
