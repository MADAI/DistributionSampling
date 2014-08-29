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
#include "ApplicationUtilities.h"

#include <algorithm> // std::transform
#include <cassert>
#include <cmath>

#include "Defaults.h"
#include "Paths.h"
#include "RuntimeParameterFileReader.h"
#include "Sampler.h"
#include "System.h"
#include "WindowsWarnings.h"

#include <boost/algorithm/string.hpp>

#include <madaisys/SystemTools.hxx>

using madaisys::SystemTools;


namespace madai {

void EnsurePathSeparatorAtEnd( std::string & path )
{
  if ( *(path.end() - 1) != madai::Paths::SEPARATOR ) {
    path.push_back( madai::Paths::SEPARATOR );
  }
}

/** General-purpose function for getting a file or directory relative
* to the statistics directory. */
std::string GetStatisticsDirectoryRelativePath(
  const std::string & statisticsDirectory,
  const RuntimeParameterFileReader & settings,
  const std::string & settingName,
  const std::string & settingDefault )
{
  std::string path = settings.GetOption( settingName, settingDefault );

  // Check for quotes around directory name
  if ( ( path[0] == '"'  && *(path.end()-1) == '"' ) ||
       ( path[0] == '\'' && *(path.end()-1) == '\'' ) ) {
    // Truncate path to remove quotes
    path = path.substr( 1, path.size()-2 );
  }

  std::string statisticsDirectoryCopy( statisticsDirectory );
  EnsurePathSeparatorAtEnd( statisticsDirectoryCopy );

  if ( path[0] != Paths::SEPARATOR ) {
    path.insert( 0, statisticsDirectoryCopy );
  }

  return path;
}


std::string GetModelOutputDirectory( const std::string & statisticsDirectory,
                                     const RuntimeParameterFileReader & settings )
{
  return GetStatisticsDirectoryRelativePath(
    statisticsDirectory, settings,
    "MODEL_OUTPUT_DIRECTORY",
    Defaults::MODEL_OUTPUT_DIRECTORY );
}

std::string GetExperimentalResultsFile( const std::string & statisticsDirectory,
                                        const RuntimeParameterFileReader & settings )
{
  return GetStatisticsDirectoryRelativePath(
    statisticsDirectory, settings,
    "EXPERIMENTAL_RESULTS_FILE",
    Defaults::EXPERIMENTAL_RESULTS_FILE );
}

std::string GetInactiveParametersFile( const std::string & statisticsDirectory,
                                       const RuntimeParameterFileReader & settings )
{
  if ( settings.GetOption( "SAMPLER_INACTIVE_PARAMETERS_FILE", "" ) == "" ) {
    return std::string();
  }

  return GetStatisticsDirectoryRelativePath(
    statisticsDirectory, settings,
    "SAMPLER_INACTIVE_PARAMETERS_FILE",
    Defaults::SAMPLER_INACTIVE_PARAMETERS_FILE );
}

std::string GetPosteriorAnalysisDirectory( const std::string & statisticsDirectory,
                                           const RuntimeParameterFileReader & settings )
{
  return GetStatisticsDirectoryRelativePath(
    statisticsDirectory, settings,
    "POSTERIOR_ANALYSIS_DIRECTORY",
    Defaults::POSTERIOR_ANALYSIS_DIRECTORY );
}

bool IsTraceCompressed( const std::string & traceFile )
{
    std::ifstream file(traceFile.c_str(), std::ios_base::in | std::ios_base::binary);
    char firstByte;
    file.read(&firstByte, 1);
    file.seekg(0, file.beg);

    // this indicates that it's not compressed
    if(firstByte == '"') {
      return false;
    }
    return true;
}

bool IsFile( const char * path )
{
  return ( SystemTools::FileExists( path ) &&
           !SystemTools::FileIsDirectory( path ) );
}

bool IsFile( const std::string & path )
{
  return IsFile( path.c_str() );
}

bool IsDirectory( const char * path )
{
  return ( SystemTools::FileExists( path ) &&
           SystemTools::FileIsDirectory( path ) );
}

bool IsDirectory( const std::string & path )
{
  return IsDirectory( path.c_str() );
}

std::string LowerCase( const char * buffer )
{
  std::string outBuffer( buffer );

  return LowerCase( outBuffer );
}

std::string LowerCase( const std::string & s )
{
  std::string copy( s );
  std::transform( copy.begin(), copy.end(),
                  copy.begin(), ::tolower );

  return copy;
}

std::vector< std::string > SplitString( const std::string & input, char separator )
{
  std::vector< std::string > tokens;
  size_t firstSeparator = input.find_first_of( separator );
  std::string token = input.substr( 0, firstSeparator );
  tokens.push_back( token );
  while ( firstSeparator != std::string::npos ) {
    size_t nextSeparator = input.find_first_of( separator, firstSeparator+1 );
    token = input.substr( firstSeparator+1,
                          (nextSeparator - firstSeparator - 1) );
    tokens.push_back( token );
    firstSeparator = nextSeparator;
  }

  return tokens;
}

std::vector< std::string > ReadLineAsTokens( std::istream & is,
                                             std::string & line )
{
  std::vector< std::string > tokens;

  std::getline( is, line );

  // First, split on comment character
  std::vector< std::string > contentAndComments;
  boost::split( contentAndComments, line, boost::is_any_of("#") );

  if ( contentAndComments.size() == 0 ) {
    return tokens;
  }

  // Trim any leading or trailing spaces in the content
  boost::trim( contentAndComments[0] );

  // Next, split only the non-comment content
  boost::split( tokens, contentAndComments[0], boost::is_any_of("\t "),
                boost::token_compress_on );

  // If first token is empty string, remove it and return
  if ( tokens[0] == "" ) {
    tokens.erase( tokens.begin() );
    return tokens;
  }

  return tokens;
}

/**
   Load a file with experimental observations in it.  The model will
   be compared against this. */
Model::ErrorType LoadObservations(Model * model, std::istream & i)
{
  if ( !i.good() ) {
    return madai::Model::FILE_NOT_FOUND_ERROR;
  }

  const std::vector< std::string > & scalarOutputNames = model->GetScalarOutputNames();
  unsigned int numberOfScalarOutputs = model->GetNumberOfScalarOutputs();
  assert(scalarOutputNames.size() == numberOfScalarOutputs);
  assert(numberOfScalarOutputs > 0);

  // Assume unset values and covariance are all zero.
  std::vector< double > observedScalarValues(numberOfScalarOutputs, 0.0);
  std::vector< double > observedScalarCovariance(
      numberOfScalarOutputs * numberOfScalarOutputs, 0.0);

  for (unsigned int j = 0; j < numberOfScalarOutputs; ++j) {
    observedScalarCovariance[j * (1 + numberOfScalarOutputs)] = 1.0;
  }

  // Keep track of which scalar names haven't yet been read
  std::set< std::string > scalarNamesRemaining;
  scalarNamesRemaining.insert(scalarOutputNames.begin(), scalarOutputNames.end());

  while (!i.eof()) { // will loop while there is still content
                      // remaining in the file
    std::string line;
    std::vector< std::string > tokens = ReadLineAsTokens( i, line );

    if ( tokens.size() == 0 )
      continue;

    std::string formatMessage( "<observed scalar name> <observed scalar value> "
                               "<observed scalar variance>" );
    if ( tokens.size() < 3 ) {
      std::cerr << "Too few tokens in line '" << line << "' of experimental "
                << "results file. Format should be " << formatMessage << "\n";
      return madai::Model::OTHER_ERROR;
    } else if (tokens.size() > 3) {
      std::cerr << "Too many tokens in line '" << line << "' of experimental "
                << "results file. Format should be " << formatMessage << "\n";
    }

    std::string name = tokens[0];
    double value = atof( tokens[1].c_str() );
    double uncertainty = atof( tokens[2].c_str() );

    int index = madai::FindIndex(scalarOutputNames, name);
    if (index != -1) {
      observedScalarValues[index] = value;

      // observedScalarCovariance is a square matrix;
      // uncertainty^2 is variance.
      observedScalarCovariance[index * (1 + numberOfScalarOutputs)] = std::pow(uncertainty, 2);

      scalarNamesRemaining.erase(name);
    } else {
      std::cout << "Unknown observed scalar name '" << name << "'. Ignoring.\n";
    }
  }

  // Report any observed scalars with unread values
  for ( std::set< std::string >::iterator iter = scalarNamesRemaining.begin();
        iter != scalarNamesRemaining.end(); ++iter ) {
    // Suppress the warning for the special case of it being the log likelihood
    std::string nameLower = *iter;
    std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
    if (nameLower != "log_likelihood" && nameLower != "loglikelihood") {
      std::cout << "Value for observed scalar '" << *iter << "' was not "
                << "specified. Assuming its value is zero.\n";
    }
  }

  Model::ErrorType e;
  e = model->SetObservedScalarValues(observedScalarValues);
  if (e != madai::Model::NO_ERROR) {
    std::cerr << "Error in Model::SetObservedScalarValues\n";
    return e;
  }

  e = model->SetObservedScalarCovariance(observedScalarCovariance);
  if (e != madai::Model::NO_ERROR) {
    std::cerr << "Error in Model::SetObservedScalarCovariance\n";
    return e;
  }
  return madai::Model::NO_ERROR;
}

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
    bool verbose )
{
  if (inactiveParametersFile == "")
    return true; // an empty filename is taken to mean no parameters
                 // should be deactivated.
  if (! IsFile(inactiveParametersFile)) {
    std::cerr
      << "Expected \"" << inactiveParametersFile
      << "\" to be a file, but it does not exist or is a directory.\n";
    return false;
  }

  madai::RuntimeParameterFileReader settings;
  if (! settings.ParseFile( inactiveParametersFile )) {
    std::cerr
      << "Error in RuntimeParameterFileReader::ParseFile("
      << inactiveParametersFile << ")\n";
    return false;
  }
  const std::vector< Parameter > & parameters = sampler.GetParameters();
  std::vector< double > parameterValues(sampler.GetCurrentParameters());
  assert(parameterValues.size() == parameters.size());

  for (unsigned int i = 0; i < parameters.size(); ++i) {
    const std::string & parameterName = parameters[i].m_Name;
    if (settings.HasOption(parameterName)) {
      parameterValues[i] = settings.GetOptionAsDouble(parameterName);
      sampler.DeactivateParameter( i );
      if ( verbose ) {
        std::cout << "Deactivating parameter '" << parameterName << "'.\n";
      }
    }
  }
  if (sampler.SetParameterValues( parameterValues) !=
      madai::Sampler::NO_ERROR) {
    std::cerr << "Error in madai::Sampler::SetParameterValues():\n";
    return false;
  }
  return true;
}


} // end namespace madai
