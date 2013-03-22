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

#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "GaussianDistribution.h"
#include "LatinHypercubeGenerator.h"
#include "Parameter.h"
#include "UniformDistribution.h"

#include "madaisys/SystemTools.hxx"


static const char usage[] =
  "usage:\n"
  "  generateTrainingPoints [options] ParametersFile OutputDirectory\n"
  "\n"
  "ParametersFile is a file containing a list of parameters and their ranges\n"
  "\n"
  "  The file should be a text file with a separate line for each parameter\n"
  "  listing the name of the parameter followed by a numeric minimum and maximum\n"
  "  defining the range of the parameter, such as\n"
  "\n"
  "  temperature 245.2 288.0\n"
  "  pressure 4.7 5.9\n"
  "  ...\n"
  "\n"
  "OutputDirectory is a path to the name of the directory containing the parameter files\n"
  "\n"
  "Options:\n"
  "\n"
  "  -h,--help                          Display help message.\n"
  "\n"
  "  -v,--verbose                       Print extra information.\n"
  "\n"
  "  -f,--format {directories,emulator} Select the output format."
  "\n"
  "  -p,--percentile                    Partition space by parameter prior\n"
  "                                     distribution percentile.\n"
  "\n"
  "  -s,--stddev <value>                Number of standard deviations from\n"
  "                                     mean of a parameter's Gaussian\n"
  "                                     distributed prior to include in the\n"
  "                                     sampling.\n"
  "\n";

typedef enum {
  DIRECTORIES_FORMAT, // Pratt format
  EMULATOR_FORMAT     // GaussianProcessEmulator format
} FormatType;


struct CommandLineOptions {
  bool verbose;
  const char * parametersFile;  // First non-flag argument.
  const char * outputDirectory; // Second non-flag argument.
  FormatType formatType;        // Type of the output
  bool partitionSpaceByPercentile;
  double standardDeviations;
};


std::string LowerCase( char * buffer )
{
  std::string outBuffer( buffer );

  std::transform( outBuffer.begin(), outBuffer.end(),
                  outBuffer.begin(), ::tolower );

  return outBuffer;
}


bool ParseCommandLineOptions( int argc, char* argv[], struct CommandLineOptions & options)
{
  // Default values for command-line options
  options.verbose = false;
  options.parametersFile = NULL;
  options.outputDirectory = NULL;
  options.formatType = DIRECTORIES_FORMAT;
  options.partitionSpaceByPercentile = false;
  options.standardDeviations = 3.0;

  // Parse options
  int argIndex;
  for ( argIndex = 1; argIndex < argc; ++argIndex ) {
    std::string argString( argv[ argIndex ] );

    if ( argString == "-v" || argString == "--verbose" ) {
      options.verbose = true;
    } else if ( argString == "-h" || argString == "--help" ) {
      std::cerr << usage;
      return false;
    } else if ( argString == "-f" || argString == "--format" ) {
      if ( argIndex + 1 < argc ) {
        std::string argString2 = LowerCase( argv[ argIndex + 1 ] );
        if ( argString2 == "directories" ) {
          options.formatType = DIRECTORIES_FORMAT;
        } else if ( argString2 == "emulator" ) {
          options.formatType = EMULATOR_FORMAT;
        } else {
          std::cerr << "Unknown format type '" << argString2 << "' provided to '"
                    << argString << "'. Expected 'directories' or 'emulator'.\n";
          return false;
        }
        argIndex++;
      } else {
        std::cerr << "No format type provided to argument '" << argString
                  << "'. Expected 'directories' or 'emulator'.\n";
        return false;
      }
    } else if ( argString == "-p" || argString == "--percentile" ) {
      options.partitionSpaceByPercentile = true;
    } else if ( argString == "-s" || argString == "--stddev" ) {
      if ( argIndex + 1 < argc ) {
        argIndex++;
        std::string argString2( argv[ argIndex ] );
        options.standardDeviations = atof( argString2.c_str() );
        if ( options.standardDeviations == 0.0 ) {
          std::cerr << "Standard deviations set to 0.0 is not valid. "
                    << "Did you supply a number to '" << argString
                    << "'?\n";
          return false;
        }
        std::cout << "argString2: " << argString2 << options.standardDeviations << std::endl;
      } else {
        std::cerr << "Missing value for '" << argString << "'.\n";
        return false;
      }
    } else {
      break; // No switch
    }
  }

  if ( argIndex < argc ) {
    options.parametersFile = argv[ argIndex++ ];
  } else {
    std::cerr << usage;
    return false;
  }

  if ( argIndex < argc ) {
    options.outputDirectory = argv[ argIndex++ ];
  } else {
    std::cerr << usage;
    return false;
  }

  if ( argIndex < argc ) {
    std::cerr << "Extra arguments after '" << options.outputDirectory << "'\n";
    return false;
  }

  // Everything went well
  return true;
}


bool ReadParameters( const struct CommandLineOptions & options,
                     std::vector< madai::Parameter > & parameters )
{
  parameters.clear();

  FILE *fp = fopen( options.parametersFile, "r" );
  if ( !fp ) {
    return false;
  }

  if ( options.verbose ) {
    std::cout << "Opened file '" << options.parametersFile << "'\n";
  }

  char buffer[4096];

  // Parse each parameter as it is listed on a line
  while ( 1 == fscanf( fp, "%4095s", buffer ) ) {
    std::string parameterName( buffer );

    if ( options.verbose ) {
      std::cout << "Read parameter '" << parameterName << "' ";
    }

    if ( 1 != fscanf( fp, "%4095s", buffer ) ) {
      std::cerr << "Could not read distribution type\n";
      fclose( fp );
      return false;
    }

    std::string distributionType( LowerCase( buffer ) );

    if ( options.verbose ) {
      std::cout << "which is of type '" << distributionType << "' ";
    }

    madai::Distribution *distribution = NULL;
    madai::UniformDistribution uniform;
    madai::GaussianDistribution gaussian;

    if ( distributionType == "uniform" ) {
      // Parse minimum and maximum values
      double minimum, maximum;
      if ( 2 != fscanf( fp, "%lf %lf", &minimum, &maximum ) ) {
        std::cerr << "Could not read uniform distribution minimum and maximum\n";
        fclose( fp );
        return false;
      }

      if ( options.verbose ) {
        std::cout << "with range [" << minimum << ", " << maximum << "]\n";
      }

      uniform.SetMinimum( minimum );
      uniform.SetMaximum( maximum );
      distribution = &uniform;

    } else if ( distributionType == "gaussian" ) {
      // Parse mean and standard deviation
      double mean, standardDeviation;
      if ( 2 != fscanf( fp, "%lf %lf", &mean, &standardDeviation ) ) {
        std::cerr << "Could not read Gaussian distribution mean and standard deviation\n";
        fclose( fp );
        return false;
      }

      if ( options.verbose ) {
        std::cout << "with mean " << mean << " and standard deviation "
                  << standardDeviation << "\n";
      }

      gaussian.SetMean( mean );
      gaussian.SetStandardDeviation( standardDeviation );
      distribution = &gaussian;
    } else {
      std::cerr << "Unknown distribution type '" << distributionType << "'\n";
      fclose( fp );
      return false;
    }

    parameters.push_back( madai::Parameter( parameterName, *distribution ) );
  }

  fclose( fp );

  // Everything went okay
  return true;
}


bool WriteDirectoriesFormat( const struct CommandLineOptions & options,
                             const std::vector< madai::Parameter > & parameters,
                             const std::vector< madai::Sample > & samples ) {
  // Create the directory structure
  bool directoryCreated = madaisys::SystemTools::MakeDirectory( options.outputDirectory );
  if ( !directoryCreated ) {
    std::cerr << "Could not create directory '" << options.outputDirectory
              << "'\n";
    return false;
  }

  std::string directory( options.outputDirectory );
  std::string experimental_results( directory + "/experimental_results" );
  std::string model_outputs( directory + "/model_outputs" );
  std::string statistical_analysis( directory + "/statistical_analysis" );

  directoryCreated = directoryCreated &&
    madaisys::SystemTools::MakeDirectory( experimental_results.c_str() );
  directoryCreated = directoryCreated &&
    madaisys::SystemTools::MakeDirectory( model_outputs.c_str() );
  directoryCreated = directoryCreated &&
    madaisys::SystemTools::MakeDirectory( statistical_analysis.c_str() );

  // Now create the run directories
  for ( size_t i = 0; i < samples.size(); ++i ) {
    const madai::Sample & sample = samples[i];

    char buffer[128];
    sprintf( buffer, "/model_outputs/run%04d", static_cast<int>( i ) );
    std::string runDirectory( directory + std::string( buffer ) );

    std::cout << runDirectory << "\n";

    directoryCreated = directoryCreated &&
      madaisys::SystemTools::MakeDirectory( runDirectory.c_str() );

    // Write the parameters to the parameters.dat file
    std::string parametersFile( runDirectory + "/parameters.dat" );
    FILE *fp = fopen( parametersFile.c_str(), "w" );
    if ( !fp ) {
      std::cerr << "Could not open file '" << parametersFile << "'\n";
      return false;
    }

    assert( parameters.size() == sample.m_ParameterValues.size() );
    for ( size_t j = 0; j < parameters.size(); ++j ) {
      fprintf( fp, "%s %f\n",
               parameters[ j ].m_Name.c_str(),
               sample.m_ParameterValues[ j ] );
    }

    fclose( fp );
  }

  return true;
}


bool WriteEmulatorFormat( const struct CommandLineOptions & options,
                          const std::vector< madai::Parameter > & parameters,
                          const std::vector< madai::Sample > & samples ) {
  // Create the output directory
  bool directoryCreated = madaisys::SystemTools::MakeDirectory( options.outputDirectory );
  if ( !directoryCreated ) {
    std::cerr << "Could not create directory '" << options.outputDirectory
              << "'\n";
    return false;
  }

  // Open the output file
  std::string outputFile( options.outputDirectory );
  outputFile += "/emulator.dat";
  FILE * fp = fopen( outputFile.c_str(), "w" );
  if ( !fp ) {
    std::cerr << "Could not create output file '" << outputFile << "'\n";
    return false;
  }

  // Print VERSION
  fprintf( fp, "VERSION 1\n" );

  // Print the PARAMETERS
  fprintf( fp, "PARAMETERS\n" );

  for ( size_t i = 0; i < parameters.size(); ++i ) {
    fprintf( fp, "%s ", parameters[i].m_Name.c_str() );
    const madai::Distribution * priorDistribution =
      parameters[i].GetPriorDistribution();
    const madai::UniformDistribution * uniformDistribution =
      dynamic_cast< const madai::UniformDistribution *>( priorDistribution );
    const madai::GaussianDistribution * gaussianDistribution =
      dynamic_cast< const madai::GaussianDistribution *>( priorDistribution );
    if ( uniformDistribution ) {
      fprintf( fp, "UNIFORM %f %f\n",
               uniformDistribution->GetMinimum(),
               uniformDistribution->GetMaximum() );
    } else if ( gaussianDistribution ) {
      fprintf( fp, "GAUSSIAN %f %f\n",
               gaussianDistribution->GetMean(),
               gaussianDistribution->GetStandardDeviation() );
    } else {
      std::cerr << "Unknown prior type\n";
      return false;
    }
  }

  // Print training points
  fprintf( fp, "NUMBER_OF_TRAINING_POINTS %d\n",
           static_cast< int >( samples.size() ) );

  // Print parameter values at training points
  fprintf( fp, "PARAMETER_VALUES\n" );

  for ( size_t i = 0; i < samples.size(); ++i ) {
    const madai::Sample & sample = samples[i];

    for ( size_t j = 0; j < sample.m_ParameterValues.size(); ++j ) {
      fprintf( fp, "%f ", sample.m_ParameterValues[j] );
    }
    fprintf( fp, "\n" );
  }

  fclose( fp );

  return true;
}


int main( int argc, char * argv[] ) {
  struct CommandLineOptions options;
  if ( !ParseCommandLineOptions( argc, argv, options ) ) {
    return EXIT_FAILURE;
  }

  if ( options.verbose ) {
    std::cout << "Options: \n";
    std::cout << "  --verbose: " << options.verbose << "\n";
    std::cout << "  --format: ";
    if ( options.formatType == DIRECTORIES_FORMAT ) std::cout << "directories\n";
    if ( options.formatType == EMULATOR_FORMAT ) std::cout << "emulator\n";
    std::cout << "  --percentile: " << options.partitionSpaceByPercentile << "\n";
    std::cout << "  --stddev: " << options.standardDeviations << "\n";
    std::cout << "  ParametersFile: " << options.parametersFile << "\n";
    std::cout << "  OutputDirectory: " << options.outputDirectory << "\n";
  }

  // Read in parameter priors
  std::vector< madai::Parameter > parameters;
  bool parametersRead = ReadParameters( options, parameters );
  if ( !parametersRead ) {
    std::cerr << "Could not read parameters from prior file '"
              << options.parametersFile << "'" << std::endl;
    return EXIT_FAILURE;
  }

  // Create the Latin hypercube sampling
  madai::LatinHypercubeGenerator sampleGenerator;
  sampleGenerator.SetStandardDeviations( options.standardDeviations );
  sampleGenerator.SetPartitionSpaceByPercentile( options.partitionSpaceByPercentile );

  std::vector< madai::Sample > samples = sampleGenerator.Generate( 100, parameters );

  if ( options.formatType == DIRECTORIES_FORMAT ) {
    WriteDirectoriesFormat( options, parameters, samples );
  } else if ( options.formatType == EMULATOR_FORMAT ) {
    WriteEmulatorFormat( options, parameters, samples );
  } else {
    std::cerr << "Unknown output format type" << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

