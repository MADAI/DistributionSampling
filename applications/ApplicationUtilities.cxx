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

  // Check for quotes around directory name
  if ( ( modelOutputDirectory[0] == '"'  && *(modelOutputDirectory.end()-1) == '"' ) ||
       ( modelOutputDirectory[0] == '\'' && *(modelOutputDirectory.end()-1) == '\'' ) ) {
    // Truncate path to remove quotes
    modelOutputDirectory = modelOutputDirectory.substr( 1, modelOutputDirectory.size()-2 );
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

  // Check for quotes around directory name
  if ( ( experimentalResultsDirectory[0] == '"' &&
         *(experimentalResultsDirectory.end()-1) == '"' ) ||
       ( experimentalResultsDirectory[0] == '\'' &&
         *(experimentalResultsDirectory.end()-1) == '\'' ) ) {
    // Truncate path to remove quotes
    experimentalResultsDirectory =
      experimentalResultsDirectory.substr( 1, experimentalResultsDirectory.size()-2 );
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

std::vector< std::string > SplitString( const std::string & input, char separator )
{
  std::vector< std::string > tokens;
  size_t firstSeparator = input.find_first_of( separator );
  std::string token = input.substr( 0, firstSeparator );
  tokens.push_back( token );
  while ( firstSeparator != std::string::npos ) {
    size_t nextSeparator = input.find_first_of( ' ', firstSeparator+1 );
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
  // std::ifstream i("DIRECTORY/experimental_results/results.dat");
  const std::vector< std::string > & scalarOutputNames = model->GetScalarOutputNames();
  unsigned int numberOfScalarOutputs = model->GetNumberOfScalarOutputs();
  assert(scalarOutputNames.size() == numberOfScalarOutputs);
  assert (numberOfScalarOutputs > 0);
  std::vector< double > observedScalarValues(numberOfScalarOutputs, 0.0);
  std::vector< double > observedScalarCovariance(
      numberOfScalarOutputs * numberOfScalarOutputs, 0.0);
  for (unsigned int j = 0; j < numberOfScalarOutputs; ++j)
    observedScalarCovariance[j * (1 + numberOfScalarOutputs)] = 1.0;
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


} // end namespace madai
