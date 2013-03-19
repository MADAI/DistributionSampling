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

#include <cstdlib>
#include <iostream>

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

struct cmdLineOpts {
  bool verbose;
  const char * parametersFile;  // First non-flag argument.
  const char * outputDirectory; // Second non-flag argument.
};


bool parseCommandLineOptions( int argc, char* argv[], struct cmdLineOpts & opts)
{
  // Default values for command-line options
  opts.verbose = false;
  opts.parametersFile = NULL;
  opts.outputDirectory = NULL;

  // Parse options
  int argIndex;
  for ( argIndex = 1; argIndex < argc; ++argIndex ) {
    std::string argString( argv[ argIndex ] );

    if ( argString == "-v" || argString == "--verbose" ) {
      opts.verbose = true;
    } else if ( argString == "-h" || argString == "--help" ) {
      std::cerr << usage;
      return false;
    } else {
      break; // No switch
    }
  }

  if ( argIndex < argc ) {
    opts.parametersFile = argv[ argIndex++ ];
  } else {
    std::cerr << usage;
    return false;
  }

  if ( argIndex < argc ) {
    opts.outputDirectory = argv[ argIndex++ ];
  } else {
    std::cerr << usage;
    return false;
  }

  // Everything went well
  return true;
}


int main( int argc, char * argv[] ) {
  struct cmdLineOpts options;
  if ( !parseCommandLineOptions( argc, argv, options ) ) {
    return EXIT_FAILURE;
  }

  if ( options.verbose ) {
    std::cout << "Options: \n";
    std::cout << "  --verbose: " << options.verbose << "\n";
    std::cout << "  ParametersFile: " << options.parametersFile << "\n";
    std::cout << "  OutputDirectory: " << options.outputDirectory << "\n";
  }

  return EXIT_SUCCESS;
}

