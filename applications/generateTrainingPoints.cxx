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
#include "Parameter.h"
#include "UniformDistribution.h"


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
  "  -h,--help    Display help message\n"
  "\n"
  "  -v,--verbose Print extra information\n"
  "\n";

struct CommandLineOptions {
  bool verbose;
  const char * parametersFile;  // First non-flag argument.
  const char * outputDirectory; // Second non-flag argument.
};


bool ParseCommandLineOptions( int argc, char* argv[], struct CommandLineOptions & options)
{
  // Default values for command-line options
  options.verbose = false;
  options.parametersFile = NULL;
  options.outputDirectory = NULL;

  // Parse options
  int argIndex;
  for ( argIndex = 1; argIndex < argc; ++argIndex ) {
    std::string argString( argv[ argIndex ] );

    if ( argString == "-v" || argString == "--verbose" ) {
      options.verbose = true;
    } else if ( argString == "-h" || argString == "--help" ) {
      std::cerr << usage;
      return false;
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


std::string LowerCase( char * buffer )
{
  std::string outBuffer( buffer );

  std::transform( outBuffer.begin(), outBuffer.end(),
                  outBuffer.begin(), ::tolower );

  return outBuffer;
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


int main( int argc, char * argv[] ) {
  struct CommandLineOptions options;
  if ( !ParseCommandLineOptions( argc, argv, options ) ) {
    return EXIT_FAILURE;
  }

  if ( options.verbose ) {
    std::cout << "Options: \n";
    std::cout << "  --verbose: " << options.verbose << "\n";
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


  return EXIT_SUCCESS;
}

