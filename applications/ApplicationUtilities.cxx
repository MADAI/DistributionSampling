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

#include <madaisys/SystemTools.hxx>

using madaisys::SystemTools;


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
  std::string modelOutputDirectory = settings.GetOption(
      "MODEL_OUTPUT_DIRECTORY", Defaults::MODEL_OUTPUT_DIRECTORY);

  // Check for quotes around directory name
  if ( ( modelOutputDirectory[0] == '"'  && *(modelOutputDirectory.end()-1) == '"' ) ||
       ( modelOutputDirectory[0] == '\'' && *(modelOutputDirectory.end()-1) == '\'' ) ) {
    // Truncate path to remove quotes
    modelOutputDirectory = modelOutputDirectory.substr( 1, modelOutputDirectory.size()-2 );
  }

  std::string statisticsDirectoryCopy( statisticsDirectory );
  EnsurePathSeparatorAtEnd( statisticsDirectoryCopy );

  if ( modelOutputDirectory[0] != Paths::SEPARATOR ) {
    modelOutputDirectory.insert( 0, statisticsDirectoryCopy );
  }

  return modelOutputDirectory;
}

std::string GetExperimentalResultsFile( const std::string & statisticsDirectory,
                                        const RuntimeParameterFileReader & settings )
{
  std::string experimentalResultsFile = settings.GetOption(
      "EXPERIMENTAL_RESULTS_FILE", Defaults::EXPERIMENTAL_RESULTS_FILE);

  // Check for quotes around directory name
  if ( ( experimentalResultsFile[0] == '"' &&
         *(experimentalResultsFile.end()-1) == '"' ) ||
       ( experimentalResultsFile[0] == '\'' &&
         *(experimentalResultsFile.end()-1) == '\'' ) ) {
    // Truncate path to remove quotes
    experimentalResultsFile =
      experimentalResultsFile.substr( 1, experimentalResultsFile.size()-2 );
  }

  std::string statisticsDirectoryCopy( statisticsDirectory );
  EnsurePathSeparatorAtEnd( statisticsDirectoryCopy );

  if ( experimentalResultsFile[0] != Paths::SEPARATOR ) {
    experimentalResultsFile.insert( 0, statisticsDirectoryCopy );
  }

  return experimentalResultsFile;
}

std::string GetInactiveParametersFile( const std::string & statisticsDirectory,
                                       const RuntimeParameterFileReader & settings )
{
  std::string inactiveParameterFile = settings.GetOption(
      "SAMPLER_INACTIVE_PARAMETERS_FILE",
      Defaults::SAMPLER_INACTIVE_PARAMETERS_FILE);
  if ( inactiveParameterFile == "" ) {
    return std::string();
  }

  // Check for quotes around directory name
  if ( ( inactiveParameterFile[0] == '"' &&
         *(inactiveParameterFile.end()-1) == '"' ) ||
       ( inactiveParameterFile[0] == '\'' &&
         *(inactiveParameterFile.end()-1) == '\'' ) ) {
    // Truncate path to remove quotes
    inactiveParameterFile =
      inactiveParameterFile.substr( 1, inactiveParameterFile.size()-2 );
  }

  std::string statisticsDirectoryCopy( statisticsDirectory );
  EnsurePathSeparatorAtEnd( statisticsDirectoryCopy );

  if ( inactiveParameterFile[0] != Paths::SEPARATOR ) {
    inactiveParameterFile.insert( 0, statisticsDirectoryCopy );
  }

  return inactiveParameterFile;
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
  std::vector< double > observedScalarValues(numberOfScalarOutputs, 0.0);
  std::vector< double > observedScalarCovariance(
      numberOfScalarOutputs * numberOfScalarOutputs, 0.0);
  for (unsigned int j = 0; j < numberOfScalarOutputs; ++j) {
    observedScalarCovariance[j * (1 + numberOfScalarOutputs)] = 1.0;
  }
  while (true) { // will loop forever if input stream lasts forever.
    std::string name;
    double value, uncertainty;
    if(! (i >> name >> value >> uncertainty))
      break;
    int index = madai::FindIndex(scalarOutputNames, name);
    if (index != -1) {
      observedScalarValues[index] = value;
      // observedScalarCovariance is a square matrix;
      observedScalarCovariance[index * (1 + numberOfScalarOutputs)] = std::pow(uncertainty, 2);
      // uncertainty^2 is variance.
    }
  }
  // assume extra values are all zero.
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
