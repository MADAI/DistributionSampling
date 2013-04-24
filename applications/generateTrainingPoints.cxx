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

#include <iostream> // std::cerr
#include <algorithm> // std::transform
#include <fstream> // std::ifstream, std::ofstream
#include <string> // std::string
#include <vector> // std::vector
#include <sstream> // std::ostringstream

#include "GaussianDistribution.h"
#include "LatinHypercubeGenerator.h"
#include "RuntimeParameterFileReader.h"
#include "ApplicationUtilities.h"
#include "Parameter.h"
#include "UniformDistribution.h"
#include "Paths.h"

#include "madaisys/SystemTools.hxx"


static const char useage[] =
  "Useage:\n"
  "  generateTrainingPoints StatisticsDirectory\n"
  "\n"
  "StatisticsDirectory is the directory in which all statistical data will\n"
  "be stored. Contains the paameter file stat_params.dat\n"
  "\n"
  "Format of and parameters which can be set in stat_params.dat:\n"
  "MODEL_OUTPUT_DIRECTORY <value>\n"
  "EXPERIMENTAL_RESULTS_DIRECTORY <value>\n"
  "NUMBER_TRAINING_POINTS <values>\n"
  "GENERATE_TRAINING_POINTS_VERBOSE <value>\n"
  "GENERATE_TRAINING_POINTS_PERCENTILE_PARTITION <value>\n"
  "GENERATE_TRAINING_POINTS_NUMBER_STDDEV <value>\n"
  "\n"
  "Default values (if not specified):\n"
  "MODEL_OUTPUT_DIRECTORY = model_output\n"
  "EXPERIMENTAL_RESULTS_DIRECTORY = experimental_results\n"
  "NUMBER_TRAINING_POINTS = 100\n"
  "GENERATE_TRAINING_POINTS_VERBOSE = false\n"
  "GENERATE_TRAINING_POINTS_PERCENTILE_PARTITION = false\n"
  "GENERATE_TRAINING_POINTS_NUMBER_STDDEV = 3\n"
  "\n"
  "This reads from StatisticsDirectory/parameter_priors.dat in order to\n"
  "use LHS to generate a series of parameter files on which to run a model.\n"
  "\n"
  "parameter_priors.dat entry formats:\n"
  "uniform name min max\n"
  "gaussian name mean std_dev\n"
  "\n"
  "Only uniform and gaussian distributions are available\n"
  "\n";

struct RuntimeOptions {
  bool verbose;
  bool partitionSpaceByPercentile;
  std::string ModelOutputDirectory;
  std::string ExperimentalResultsDirectory;
  double standardDeviations;
  int numberOfTrainingPoints;
};


std::string LowerCase( char * buffer )
{
  std::string outBuffer( buffer );

  std::transform( outBuffer.begin(), outBuffer.end(),
                  outBuffer.begin(), ::tolower );

  return outBuffer;
}

void LowerCase( std::string & s )
{
  std::transform( s.begin(), s.end(),
                  s.begin(), ::tolower );
}


bool ParseRuntimeOptions( int argc, char* argv[], struct RuntimeOptions & options)
{
  // Default values for command-line options
  options.verbose = false;
  options.ModelOutputDirectory = madai::Paths::DEFAULT_MODEL_OUTPUT_DIRECTORY;
  options.ExperimentalResultsDirectory = madai::Paths::DEFAULT_EXPERIMENTAL_RESULTS_DIRECTORY;
  options.partitionSpaceByPercentile = false;
  options.standardDeviations = 3.0;
  options.numberOfTrainingPoints = 100;

  // Parse options
  for ( unsigned int argIndex = 0; argIndex < argc; ++argIndex ) {
    std::string argString( argv[ argIndex ] );

    if ( argString == "GENERATE_TRAINING_POINTS_VERBOSE" ) {
      std::string tstring( argv[ argIndex + 1 ] );
      if ( tstring == "false" || tstring == "0" ) {
        options.verbose = false;
      } else if ( tstring == "true" || tstring == "1" ) {
        options.verbose = true;
      } else {
        std::cerr << "GENERATE_TRAINING_POINTS_VERBOSE given invalid argument \""
          << tstring << "\"\n";
        return false;
      }
      argIndex++;
    } else if ( argString == "GENERATE_TRAINING_POINTS_PERCENTILE_PARTITION" ) {
      std::string tstring( argv[ argIndex + 1 ] );
      if ( tstring == "false" || tstring == "0" ) {
        options.partitionSpaceByPercentile = false;
      } else if ( tstring == "true" || tstring == "1" ) {
        options.partitionSpaceByPercentile = true;
      } else {
        std::cerr << "GENERATE_TRAINING_POINTS_PERCENTILE_PARTITION given invalid argument \""
          << tstring << "\"\n";
        return false;
      }
      argIndex++;
    } else if ( argString == "GENERATE_TRAINING_POINTS_NUMBER_STDDEV" ) {
      std::string tstring( argv[ argIndex + 1 ] );
      options.standardDeviations = atof( tstring.c_str() );
      if ( options.standardDeviations <= 0.0 ) {
        std::cerr << "GENERATE_TRAINING_POINTS_NUMBER_STDDEV given invalid argument \""
          << tstring << "\"\n";
        return false;
      }
    } else if ( argString == "MODEL_OUTPUT_DIRECTORY" ) {
      options.ModelOutputDirectory = std::string( argv[ argIndex + 1 ] );
      argIndex++;
    } else if ( argString == "EXPERIMENTAL_RESULTS_DIRECTORY" ) {
      options.ExperimentalResultsDirectory = std::string( argv[ argIndex + 1 ] );
      argIndex++;
    } else if ( argString == "NUMBER_TRAINING_POINTS" ) {
      std::string tstring( argv[ argIndex + 1 ] );
      options.numberOfTrainingPoints = atoi( tstring.c_str() );
      if ( options.numberOfTrainingPoints <= 0 ) {
        std::cerr << "NUMBER_TRAINING_POINTS given invalid argument \""
          << tstring << "\"\n";
        return false;
      }
      argIndex++;
    }
  }

  // Everything went well
  return true;
}

static void discard_comments( std::istream & i, char comment_character ) {
  int c;
  while (i.good() && ((c = i.peek()) != EOF)) {
    if ((c == ' ') || (c == '\t') || ( c == '\n' )) {
      char ch;
      i.get(ch); // discard whitespace at beginning of line;
    } else if (c == comment_character) {
      std::string s;
      std::getline( i, s );
    } else {
      return; // line begins with some non-comment, non-whitespace character.
    }
  }
}

bool ReadParameters( std::string parametersFile,
                     std::vector< madai::Parameter > & parameters,
                     bool verbose )
{
  parameters.clear();

  std::ifstream parameterFile(parametersFile.c_str());
  if (! parameterFile.good()) {
    std::cerr << "[ReadParameters] Unable to open file " << parametersFile << ".";
    return false;
  }

  if ( verbose ) {
    std::cout << "Opened file '" << parametersFile << "'\n";
  }

  // Parse each parameter as it is listed on a line
  std::string parameterName;
  std::string distributionType;
  discard_comments(parameterFile, '#');
  while (parameterFile >> distributionType >> parameterName) {
    if (! parameterFile.good()) {
      parameterFile.close();
      break;
    }
    LowerCase(distributionType);
    if ( verbose ) {
      std::cout << "Read parameter '" << parameterName << "' "
                << "which is of type '" << distributionType << "' ";
    }

    madai::Distribution *distribution = NULL;
    madai::UniformDistribution uniform;
    madai::GaussianDistribution gaussian;

    if ( distributionType == "uniform" ) {
      // Parse minimum and maximum values
      double minimum, maximum;
      parameterFile >> minimum >> maximum;
      if (! parameterFile.good()) {
        std::cerr << "Could not read uniform distribution minimum and maximum\n";
        parameterFile.close();
        return false;
      }
      if ( verbose ) {
        std::cout << "with range [" << minimum << ", " << maximum << "]\n";
      }
      uniform.SetMinimum( minimum );
      uniform.SetMaximum( maximum );
      distribution = &uniform;

    } else if ( distributionType == "gaussian" ) {
      // Parse mean and standard deviation
      double mean, standardDeviation;
      parameterFile >> mean >> standardDeviation;
      if (! parameterFile.good()) {
        std::cerr << "Could not read Gaussian distribution mean and standard deviation\n";
        parameterFile.close();
        return false;
      }

      if ( verbose ) {
        std::cout << "with mean " << mean << " and standard deviation "
                  << standardDeviation << "\n";
      }

      gaussian.SetMean( mean );
      gaussian.SetStandardDeviation( standardDeviation );
      distribution = &gaussian;
    } else {
      std::cerr << "Unknown distribution type '" << distributionType << "'\n";
      parameterFile.close();
      return false;
    }
    parameters.push_back( madai::Parameter( parameterName, *distribution ) );
    discard_comments(parameterFile, '#');
  }

  parameterFile.close();
  // Everything went okay
  return true;
}


bool WriteDirectories( const std::string ModelOutputDirectory,
                       const std::string ExperimentalResultsDirectory,
                       const std::vector< madai::Parameter > & parameters,
                       const std::vector< madai::Sample > & samples,
                       const bool verbose ) {
  // Create the directory structure
  bool directoryCreated = madaisys::SystemTools::MakeDirectory( ModelOutputDirectory.c_str() );
  if ( !directoryCreated ) {
    std::cerr << "Could not create directory '" << ModelOutputDirectory << "'\n";
    return false;
  }
  
  directoryCreated = madaisys::SystemTools::MakeDirectory( ExperimentalResultsDirectory.c_str() );
  if ( !directoryCreated ) {
    std::cerr << "Could not create directory '" << ExperimentalResultsDirectory << "'\n";
    return false;
  }
  
  // Now create the run directories
  for ( size_t i = 0; i < samples.size(); ++i ) {
    const madai::Sample & sample = samples[i];
    
    std::ostringstream buffer;
    buffer << ModelOutputDirectory << "/run"
    << std::setw( 7 ) << std::setfill( '0' ) << i;
    std::string runDirectory(buffer.str());
    
    if ( verbose ) {
      std::cout << runDirectory << "\n";
    }
    
    directoryCreated = directoryCreated &&
      madaisys::SystemTools::MakeDirectory( runDirectory.c_str() );
      
    // Write the parameters to parameters.dat file
    std::string parametersFile( runDirectory + "/parameters.dat" );
    std::ofstream outfile( parametersFile.c_str() );
    if ( !outfile ) {
      std::cerr << "Could not open file '" << parametersFile << "'\n";
      return false;
    }
    
    assert( parameters.size() == sample.m_ParameterValues.size() );
    for ( size_t j = 0; j < parameters.size(); ++j ) {
      outfile << parameters[ j ].m_Name << ' '
              << sample.m_ParameterValues[ j ] << '\n';
    }
    
    outfile.close();
  }
  
  return true;
}


int main( int argc, char * argv[] ) {
  std::string StatisticsDirectory;
  std::string ModelOutputDirectory;
  std::string ExperimentalResultsDirectory;
  if ( argc > 1 ) {
    StatisticsDirectory = std::string( argv[1] );
    madai::EnsurePathSeparatorAtEnd( StatisticsDirectory );
  } else {
    std::cerr << useage << '\n';
    return EXIT_FAILURE;
  }
  madai::RuntimeParameterFileReader RPFR;
  RPFR.ParseFile( StatisticsDirectory +
                  madai::Paths::RUNTIME_PARAMETER_FILE );
  char** Args = RPFR.GetArguments();
  int NArgs = RPFR.GetNumberOfArguments();
  struct RuntimeOptions options;
  if ( !ParseRuntimeOptions( NArgs, Args, options ) ) {
    return EXIT_FAILURE;
  }

  ModelOutputDirectory = StatisticsDirectory + options.ModelOutputDirectory;
  ExperimentalResultsDirectory = StatisticsDirectory + options.ExperimentalResultsDirectory;
  if ( options.verbose ) {
    std::cout << "Options: \n";
    std::cout << "  VERBOSE: " << options.verbose << "\n";
    std::cout << "  PERCENTILE_PARTITION: " << options.partitionSpaceByPercentile << "\n";
    std::cout << "  STANDARD_DEVIATIONS: " << options.standardDeviations << "\n";
    std::cout << "  NUMBER_TRAINING_POINTS: " << options.numberOfTrainingPoints << "\n";
    std::cout << "  MODEL_OUTPUT_DIRECTORY: " << ModelOutputDirectory << "\n";
    std::cout << "  EXPERIMENTAL_RESULTS_DIRECTORY: " << ExperimentalResultsDirectory << "\n";
  }

  // Read in parameter priors
  std::vector< madai::Parameter > parameters;
  std::string parametersFile =  StatisticsDirectory +
    madai::Paths::PARAMETER_PRIORS_FILE;
  bool parametersRead = ReadParameters( parametersFile, parameters, options.verbose );
  if ( !parametersRead ) {
    std::cerr << "Could not read parameters from prior file '"
              << parametersFile << "'" << std::endl;
    return EXIT_FAILURE;
  }

  // Create the Latin hypercube sampling
  madai::LatinHypercubeGenerator sampleGenerator;
  sampleGenerator.SetStandardDeviations( options.standardDeviations );
  sampleGenerator.SetPartitionSpaceByPercentile( options.partitionSpaceByPercentile );

  std::vector< madai::Sample > samples =
    sampleGenerator.Generate( options.numberOfTrainingPoints, parameters );

  WriteDirectories( ModelOutputDirectory, ExperimentalResultsDirectory, parameters,
                    samples, options.verbose );

  return EXIT_SUCCESS;
}
