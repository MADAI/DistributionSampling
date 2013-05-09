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
#include <fstream>
#include <set>
#include <string>
#include <utility>

#include "ApplicationUtilities.h"
#include "GaussianDistribution.h"
#include "GaussianProcessEmulator.h"
#include "GaussianProcessEmulatorDirectoryReader.h"
#include "Paths.h"
#include "RuntimeParameterFileReader.h"
#include "UniformDistribution.h"

#include "madaisys/Directory.hxx"
#include "madaisys/SystemTools.hxx"

#include <boost/algorithm/string.hpp>


std::vector< std::string > getLineAsTokens( std::ifstream & is,
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


// Checks parameter names and priors. Returns true on success.
bool checkParameters( const madai::GaussianProcessEmulator & gpe,
                      const std::string & statisticsDirectory )
{
  // Load the input parameter names
  std::set< std::string > parameterNames;
  std::vector< std::string > parameterPriorTypes;
  std::vector< std::pair< double, double > > distributionParameters;
  std::string parameterPriorsFileName =
    statisticsDirectory + madai::Paths::PARAMETER_PRIORS_FILE;
  std::ifstream parameterPriorsFile( parameterPriorsFileName.c_str() );
  while ( parameterPriorsFile.good() ) {
    std::string line;
    std::vector< std::string > tokens = getLineAsTokens( parameterPriorsFile,
                                                         line );

    // Skip empty lines and comment lines
    if ( tokens.size() == 0 ) continue;

    if ( tokens.size() != 4 ) {
      std::cerr << "Parameter prior line not in expected format\n";
      std::cerr << "File: '" << parameterPriorsFileName << "'\n";
      std::cerr << "Line: '" << line << "'\n";
      std::cerr << "Tokens: " << tokens.size() << "\n";
      for ( size_t i = 0; i < tokens.size(); ++i ) {
        std::cerr << "'" << tokens[i] << "'\n";
      }

      return false;
    }
    parameterNames.insert( tokens[1] );
    boost::algorithm::to_lower( tokens[0] );
    parameterPriorTypes.push_back( tokens[0] );

    distributionParameters.push_back( std::make_pair(
                                                     atof( tokens[2].c_str() ),
                                                     atof( tokens[3].c_str() ) ) );
  }
  parameterPriorsFile.close();

  // Check input parameter names
  std::set< std::string > remainingParameterNames( parameterNames );
  for ( size_t i = 0; i < gpe.m_Parameters.size(); ++i ) {
    if ( parameterNames.count( gpe.m_Parameters[i].m_Name ) == 0 ) {
      std::cerr << "Parameter name '" << gpe.m_Parameters[i].m_Name
                << "' unexpected\n";
      return false;
    } else {
      remainingParameterNames.erase( gpe.m_Parameters[i].m_Name );
    }
  }

  if ( remainingParameterNames.size() > 0 ) {
    for ( std::set< std::string >::iterator i = remainingParameterNames.begin();
          i != remainingParameterNames.end();
          ++i ) {
      std::cerr << "Parameter '" << *i << "' not parsed by reader\n";
      return false;
    }

  }

  // Check parameter prior distributions
  for ( size_t i = 0; i < gpe.m_Parameters.size(); ++i ) {
    madai::UniformDistribution * uDistribution =
      dynamic_cast< madai::UniformDistribution * >( gpe.m_Parameters[i].m_PriorDistribution );
    madai::GaussianDistribution * gDistribution =
      dynamic_cast< madai::GaussianDistribution * >( gpe.m_Parameters[i].m_PriorDistribution );

    if ( uDistribution ) {
      if ( parameterPriorTypes[i] != "uniform" ) {
        std::cerr << "Reader parsed prior for parameter '"
                  << gpe.m_Parameters[i].m_Name << "' as uniform, but prior "
                  << "type was actually '" << parameterPriorTypes[i] << "'\n";
        return false;
      }

      if ( uDistribution->GetMinimum() != distributionParameters[i].first ) {
        std::cerr << "Reader parsed uniform distributed parameter '"
                  << gpe.m_Parameters[i].m_Name << "' minimum of "
                  << distributionParameters[i].first
                  << " but got " << uDistribution->GetMinimum() << "\n";
        return false;
      }

      if ( uDistribution->GetMaximum() != distributionParameters[i].second ) {
        std::cerr << "Reader parsed uniform distributed parameter '"
                  << gpe.m_Parameters[i].m_Name << "' maximum of "
                  << distributionParameters[i].second
                  << " but got " << uDistribution->GetMinimum() << "\n";
        return false;
      }
    }

    if ( gDistribution ) {
      if ( parameterPriorTypes[i] != "gaussian" ) {
        std::cerr << "Reader parsed prior for parameter '"
                  << gpe.m_Parameters[i].m_Name << "' as gaussian, but prior "
                  << "type was actually '" << parameterPriorTypes[i] << "'\n";
        return false;
      }

      if ( gDistribution->GetMean() != distributionParameters[i].first ) {
        std::cerr << "Reader parsed Gaussian distributed parameter '"
                  << gpe.m_Parameters[i].m_Name << "' mean of "
                  << distributionParameters[i].first
                  << " but got " << gDistribution->GetMean() << "\n";
        return false;
      }

      if ( gDistribution->GetStandardDeviation() !=
           distributionParameters[i].second ) {
        std::cerr << "Reader parsed Gaussian distributed parameter '"
                  << gpe.m_Parameters[i].m_Name << "' standard deviation of "
                  << distributionParameters[i].second
                  << " but got " << gDistribution->GetStandardDeviation() << "\n";
        return false;
      }

    }

    // \todo - Check prior distribution parameters
  }

  // Check number of parameters
  if ( gpe.m_NumberParameters != static_cast< int >( gpe.m_Parameters.size() ) ) {
    std::cerr << "m_NumberParameters != m_Parameters.size()\n";
    return false;
  }

  return true;
}


// Checks output names. Returns true on success.
bool checkOutputs( const madai::GaussianProcessEmulator & gpe,
                   const std::string & statisticsDirectory )
{
  // Load the output names
  std::set< std::string > outputNames;
  std::string outputFileName =
    statisticsDirectory + madai::Paths::OBSERVABLE_NAMES_FILE;
  std::ifstream outputFile( outputFileName.c_str() );
  while ( outputFile.good() ) {
    std::string line;
    std::vector< std::string > tokens = getLineAsTokens( outputFile, line );

    // Skip empty lines and comment lines
    if ( tokens.size() == 0 ) continue;

    outputNames.insert( tokens[0] );
  }
  outputFile.close();

  // Check output names
  std::set< std::string > remainingOutputNames( outputNames );
  for ( size_t i = 0; i < gpe.m_OutputNames.size(); ++i ) {
    if ( outputNames.count( gpe.m_OutputNames[i] ) == 0 ) {
      std::cerr << "Output name '" << gpe.m_OutputNames[i]
                << "' unexpected\n";
      return false;
    } else {
      remainingOutputNames.erase( gpe.m_OutputNames[i] );
    }
  }

  // Check number of outputs
  if ( gpe.m_NumberOutputs != static_cast< int >( gpe.m_OutputNames.size() ) ) {
    std::cerr << "m_NumberOutputs != m_OutputNames.size()\n";
    return EXIT_FAILURE;
  }

  return true;
}


std::vector< std::string > getRunDirectories( const std::string & modelOutputDirectory ) {

  std::vector< std::string > directories;

  madaisys::Directory directory;
  if ( !directory.Load( modelOutputDirectory.c_str() ) ) {
    std::cerr << "Couldn't read directory '" << modelOutputDirectory << "'\n";
    return directories;
  }

  for ( unsigned long i = 0; i < directory.GetNumberOfFiles(); ++i ) {
    std::string fileName( directory.GetFile( i ) );
    int dummy;
    if ( sscanf( fileName.c_str(), "run%d", &dummy ) == 1 ) {
      directories.push_back( fileName );
    }
  }

  std::sort( directories.begin(), directories.end() );

  return directories;
}


bool checkNumberOfTrainingPoints( const madai::GaussianProcessEmulator & gpe,
                                  const std::string & modelOutputDirectory )
{
  std::vector< std::string > directories = getRunDirectories( modelOutputDirectory );

  // Count the number of directories with parameter and results files
  int runCounter = 0;
  for ( size_t i = 0; i < directories.size(); ++i ) {
    std::string runDirectory( directories[i] );

    // Check for parameters and results files
    std::string parametersFileName( modelOutputDirectory +
                                    madai::Paths::SEPARATOR +
                                    runDirectory +
                                    madai::Paths::SEPARATOR +
                                    madai::Paths::PARAMETERS_FILE );
    std::ifstream parametersFile( parametersFileName.c_str() );
    std::string resultsFileName( modelOutputDirectory +
                                 madai::Paths::SEPARATOR +
                                 runDirectory +
                                 madai::Paths::SEPARATOR +
                                 madai::Paths::RESULTS_FILE );
    std::ifstream resultsFile( resultsFileName.c_str() );

    if ( parametersFile.good() && resultsFile.good() ) {
      runCounter++;
    }

    parametersFile.close();
    resultsFile.close();
  }

  if ( gpe.m_NumberTrainingPoints != runCounter ) {
    std::cerr << "Reader reported " << gpe.m_NumberTrainingPoints
              << " training points, but there were "
              << runCounter << "\n";
    return false;
  }

  return true;
}


// Checks that the reader read the parameter values correctly
bool checkParameterValues( const madai::GaussianProcessEmulator & gpe,
                           const std::string & modelOutputDirectory )
{
  std::vector< std::string > directories = getRunDirectories( modelOutputDirectory );

  for ( size_t i = 0; i < directories.size(); ++i ) {
    std::string runDirectory( directories[i] );

    // Check parameter files
    std::string parametersFileName( modelOutputDirectory +
                                    madai::Paths::SEPARATOR +
                                    runDirectory +
                                    madai::Paths::SEPARATOR +
                                    madai::Paths::PARAMETERS_FILE );
    std::ifstream parametersFile( parametersFileName.c_str() );

    while ( parametersFile.good() ) {
      std::string line;
      std::vector< std::string > tokens = getLineAsTokens( parametersFile,
                                                           line );

      // Skip empty lines and comment lines
      if ( tokens.size() == 0 ) continue;

      if ( tokens.size() >= 2 ) {
        std::string name = tokens[0];
        double expectedValue = atof( tokens[1].c_str() );
        for ( size_t j = 0; j < gpe.m_Parameters.size(); ++j ) {
          if ( gpe.m_Parameters[j].m_Name == name ) {
            double value = gpe.m_TrainingParameterValues( i, j );
            if ( expectedValue != value ) {
              std::cerr << "Expected parameter value " << expectedValue << " in "
                        << "gpe.m_ParameterValues(" << i << ", "
                        << j << ") but got " << value << "\n";
              return false;
            }
            continue;
          }
        }
      }
    }

    parametersFile.close();
  }

  return true;
}


// Checks that the reader read the output values correctly
bool checkOutputValues( const madai::GaussianProcessEmulator & gpe,
                        const std::string & modelOutputDirectory )
{
  std::vector< std::string > directories =
    getRunDirectories( modelOutputDirectory );

  for ( size_t i = 0; i < directories.size(); ++i ) {
    std::string runDirectory( directories[i] );
    // Check results file
    std::string resultsFileName( modelOutputDirectory +
                                 madai::Paths::SEPARATOR +
                                 runDirectory +
                                 madai::Paths::SEPARATOR +
                                 madai::Paths::RESULTS_FILE );
    std::ifstream resultsFile( resultsFileName.c_str() );

    while ( resultsFile.good() ) {
      std::string line;
      std::vector< std::string > tokens = getLineAsTokens( resultsFile, line );

      // Skip empty lines and comment lines
      if ( tokens.size() == 0 ) continue;

      if ( tokens.size() >= 2 ) {
        std::string name = tokens[0];
        double expectedValue = atof( tokens[1].c_str() );
        double uncertainty;
        if ( tokens.size() >= 3 ) {
          uncertainty = atof( tokens[2].c_str() );
        }

        for ( size_t j = 0; j < gpe.m_OutputNames.size(); ++j ) {
          if ( gpe.m_OutputNames[j] == name ) {
            double value = gpe.m_TrainingOutputValues( i, j );
            if ( expectedValue != value ) {
              std::cerr << "Expected output value " << expectedValue << " in "
                        << "gpe.m_TrainingOutputValues(" << i << ", "
                        << j << ") but got " << value << "\n";
              return false;
            }
          }
        }
      }
    }

    resultsFile.close();
  }

  return true;
}


bool checkOutputMeans( const madai::GaussianProcessEmulator & gpe )
{
  // Check output means
  for ( int i = 0; i < gpe.m_TrainingOutputMeans.size(); ++i ) {
    if ( gpe.m_TrainingOutputMeans(i) != 0.0 ) {
      std::cerr << "Training output mean was expected to be 0.0, but got "
                << gpe.m_TrainingOutputMeans << "\n";
      return false;
    }
  }

  return true;
}

bool checkOutputUncertaintyScales( const madai::GaussianProcessEmulator & gpe,
                                   const std::string & modelOutputDirectory,
                                   const std::string & experimentalResultsFileName )
{
  // Read in the uncertainty from the experimental results
  std::ifstream resultsFile( experimentalResultsFileName.c_str() );

  if ( !resultsFile.good() ) {
    std::cerr << "Could not open file '" << experimentalResultsFileName << "'\n";
    return false;
  }

  std::vector< double > experimentalUncertainties( gpe.m_NumberOutputs, 0.0 );
  while ( resultsFile.good() ) {
    std::string line;
    std::vector< std::string > tokens = getLineAsTokens( resultsFile, line );

    // Skip lines with no tokens
    if ( tokens.size() == 0 ) continue;

    if ( tokens.size() != 3 ) {
      std::cerr << "Too few tokens in line '" << line << "' in file '"
                << experimentalResultsFileName << "'\n";
      std::cerr << "Format should be <name> <value> <uncertainty>\n";
      return false;
    }

    std::string name( tokens[0] );
    double uncertainty = atof( tokens[2].c_str() );
    for ( size_t j = 0; j < gpe.m_OutputNames.size(); ++j ) {
      if ( gpe.m_OutputNames[j] == name ) {
        experimentalUncertainties[j] = uncertainty;
        break;
      }
    }
  }

  // Average the uncertainties from the model output
  std::vector< double > accumulatedUncertainties( gpe.m_NumberOutputs, 0.0 );

  std::vector< std::string > directories =
    getRunDirectories( modelOutputDirectory );

  for ( size_t i = 0; i < directories.size(); ++i ) {
    std::string runDirectory( directories[i] );

    // Parse results file
    std::string resultsFileName( modelOutputDirectory +
                                 madai::Paths::SEPARATOR +
                                 runDirectory +
                                 madai::Paths::SEPARATOR +
                                 madai::Paths::RESULTS_FILE );
    std::ifstream resultsFile( resultsFileName.c_str() );

    while ( resultsFile.good() ) {
      std::string line;
      std::vector< std::string > tokens = getLineAsTokens( resultsFile, line );

      // Skip empty lines and comment lines
      if ( tokens.size() == 0 ) continue;

      if ( tokens.size() >= 2 ) {
        std::string name = tokens[0];
        double uncertainty = 0.0;
        if ( tokens.size() >= 3 ) {
          uncertainty = atof( tokens[2].c_str() );
        }

        std::vector< std::string >::const_iterator outputNameIter =
          std::find( gpe.m_OutputNames.begin(), gpe.m_OutputNames.end(), name );

        if ( outputNameIter != gpe.m_OutputNames.end() ) {
          int index = static_cast< int >( std::distance( gpe.m_OutputNames.begin(),
                                                         outputNameIter ) );
          accumulatedUncertainties[ index ] += uncertainty;
        }
      }
    }
  }

  std::vector< double > uncertaintyScales;
  gpe.GetUncertaintyScales( uncertaintyScales );

  for ( size_t i = 0; i < accumulatedUncertainties.size(); ++i ) {
    double averagedOutputUncertainty = accumulatedUncertainties[i] /
      static_cast< double >( directories.size() );

    double outputUncertaintyScale = std::sqrt(
      std::pow( averagedOutputUncertainty, 2 ) +
      std::pow( experimentalUncertainties[i], 2 ) );

    // Add averaged model uncertainty to observed uncertainty
    if ( outputUncertaintyScale != uncertaintyScales[i] ) {
      std::cerr << "Expected m_UncertaintyScales to be "
                << outputUncertaintyScale << " but reader got "
                << uncertaintyScales[i] << "\n";
      return false;
    }
  }

  return true;
}

bool checkObservedOutputValues( const madai::GaussianProcessEmulator & gpe,
                                const std::string & experimentalResultsFileName )
{
  std::string resultsFileName( experimentalResultsFileName );
  std::ifstream resultsFile( resultsFileName.c_str() );

  if ( !resultsFile.good() ) {
    std::cerr << "Could not open file '" << resultsFileName << "'\n";
    return false;
  }

  while ( resultsFile.good() ) {
    std::string line;

    std::vector< std::string > tokens = getLineAsTokens( resultsFile, line );

    // Skip empty lines and comment lines
    if ( tokens.size() == 0 ) continue;

    if ( tokens.size() == 3 ) {
      std::string name = tokens[0];
      double expectedValue = atof( tokens[1].c_str() );

      // Find index of name
      std::vector< std::string >::const_iterator outputNameIter =
        std::find( gpe.m_OutputNames.begin(), gpe.m_OutputNames.end(), name );

      if ( outputNameIter != gpe.m_OutputNames.end() ) {
        int index = static_cast< int >( std::distance( gpe.m_OutputNames.begin(),
                                                       outputNameIter ) );
        double value = gpe.m_ObservedValues[ index ];
        if ( value != expectedValue ) {
          std::cerr << "Observed output value in emulator is " << value
                    << " but " << expectedValue << " was expected.\n";
          return false;
        }
      } else {
        std::cerr << "Output name '" << name << "' in experimental results file "
                  << "is not known to the emulator.\n";
        return false;
      }

    }

  }

  return true;
}


int main( int argc, char *argv[] )
{
  if ( argc < 2 ) {
    std::cerr << "Usage: " << argv[0] << " <StatisticsDirectory>\n";
    return EXIT_FAILURE;
  }

  std::string statisticsDirectory = std::string( argv[1] );
  madai::EnsurePathSeparatorAtEnd( statisticsDirectory );

  // Read in runtime parameters
  madai::RuntimeParameterFileReader settings;
  std::string settingsFile = statisticsDirectory +
    madai::Paths::RUNTIME_PARAMETER_FILE;
  if ( !settings.ParseFile( settingsFile ) ) {
    std::cerr << "Could not open runtime parameter file '" << settingsFile << "'\n";
    return EXIT_FAILURE;
  }

  std::string modelOutputDirectory =
    madai::GetModelOutputDirectory( statisticsDirectory, settings );
  std::string experimentalResultsFile =
    madai::GetExperimentalResultsFile( statisticsDirectory, settings );

  // Read in the training data
  madai::GaussianProcessEmulator gpe;
  madai::GaussianProcessEmulatorDirectoryReader directoryReader;

  directoryReader.SetVerbose( true );
  if ( !directoryReader.LoadTrainingData( &gpe,
                                          modelOutputDirectory,
                                          statisticsDirectory,
                                          experimentalResultsFile ) ) {
    std::cerr << "Error loading training data.\n";
    return EXIT_FAILURE;
  }

  if ( !checkParameters( gpe, statisticsDirectory ) ) {
    std::cerr << "Error in checkParameters\n";
    return EXIT_FAILURE;
  }

  if ( !checkOutputs( gpe, statisticsDirectory ) ) {
    std::cerr << "Error in checkOutputs\n";
    return EXIT_FAILURE;
  }

  if ( !checkNumberOfTrainingPoints( gpe, modelOutputDirectory ) ) {
    std::cerr << "Error in checkNumberOfTrainingPoints\n";
    return EXIT_FAILURE;
  }

  if ( !checkParameterValues( gpe, modelOutputDirectory ) ) {
    std::cerr << "Error in checkParameterValues\n";
    return EXIT_FAILURE;
  }

  if ( !checkOutputValues( gpe, modelOutputDirectory ) ) {
    std::cerr << "Error in checkOutputValues\n";
    return EXIT_FAILURE;
  }

  if ( !checkOutputMeans( gpe ) ) {
    std::cerr << "Error in checkOutputMeans\n";
    return EXIT_FAILURE;
  }

  if ( !checkOutputUncertaintyScales( gpe,
                                      modelOutputDirectory,
                                      experimentalResultsFile ) ) {
    std::cerr << "Error in checkOutputUncertaintyScales\n";
    return EXIT_FAILURE;
  }

  if ( !checkObservedOutputValues( gpe, experimentalResultsFile ) ) {
    std::cerr << "Error in checkObservedOutputValues\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
