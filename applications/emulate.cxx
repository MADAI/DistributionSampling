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

/**
emulate
  execute a N-D Gaussian Process Model Emulator

ACKNOWLEDGMENTS:
  This software was written in 2012-2013 by Hal Canary
  <cs.unc.edu/~hal>, based off of the MADAIEmulator program (Copyright
  2009-2012 Duke University) by C.Coleman-Smith <cec24@phy.duke.edu>
  in 2010-2012 while working for the MADAI project <http://madai.us/>.

USE:
  For details on how to use emulate, consult the manpage via:
    $ nroff -man < [PATH_TO/]emulate.1 | less
  or, if the manual is installed:
    $ man 1 emulate
*/

#include <iostream>
#include <fstream>
#include <cassert>
#include <cstdlib>

#include "ApplicationUtilities.h"
#include "GaussianProcessEmulator.h"
#include "GaussianDistribution.h"
#include "GaussianProcessEmulatorDirectoryReader.h"
#include "GaussianProcessEmulatorSingleFileReader.h"
#include "RuntimeParameterFileReader.h"
#include "UniformDistribution.h"
#include "Paths.h"

static const char useage [] =
  "useage:\n"
  "  emulate StatisticsDirectory\n"
  "\n"
  "StatisticsDirectory is the directory containing all statistical analysis data.\n"
  "Contains parameter file stat_params.dat\n"
  "\n"
  "Structure of stat_params:\n"
  "MODEL_OUTPUT_DIRECTORY <value>\n"
  "EXPERIMENTAL_RESULTS_DIRECTORY <value>\n"
  "EMULATE_QUIET <value>\n"
  "\n"
  "Defaults (if not specified) in order listed:\n"
  "model_output\n"
  "experimental_results\n"
  "false\n"
  "\n";

struct RuntimeOpts{
  bool quietFlag;
  std::string ModelOutputDirectory;
  std::string ExperimentalResultsDirectory;
};

/**
   Option parsing using getoptlong.  If it fails, returns false. */
bool parseRuntimeOptions(int argc, char** argv, struct RuntimeOpts & opts)
{
  // init with default values
  opts.ModelOutputDirectory = madai::Paths::DEFAULT_MODEL_OUTPUT_DIRECTORY;
  opts.ExperimentalResultsDirectory = madai::Paths::DEFAULT_EXPERIMENTAL_RESULTS_DIRECTORY;
  opts.quietFlag = false;
  
  for ( unsigned int i = 0; i < argc; i++ ) {
    std::string argString( argv[i] );
    
    if ( argString == "MODEL_OUTPUT_DIRECTORY" ) {
      opts.ModelOutputDirectory = std::string( argv[i+1] );
      i++;
    } else if ( argString == "EXPERIMENTAL_RESULTS_DIRECTORY" ) {
      opts.ExperimentalResultsDirectory = std::string( argv[i+1] );
      i++;
    } else if ( argString == "EMULTE_QUIET" ) {
      std::string tstring( argv[i+1] );
      if ( tstring == "true" || tstring == "1" ) {
        opts.quietFlag = true;
      } else if ( tstring == "false" || tstring == "0" ) {
        opts.quietFlag = false;
      } else {
        std::cerr << "Unrecognized value for parameter EMULATE_QUIET: "
                  << tstring << "\n";
        return false;
      }
      i++;
    }
  }
  return true;
}

std::ostream & operator <<(std::ostream & o, const madai::Distribution * d) {
  const madai::UniformDistribution * uniformPriorDist
    = dynamic_cast<const madai::UniformDistribution *>(d);
  const madai::GaussianDistribution * gaussianPriorDist
    = dynamic_cast<const madai::GaussianDistribution *>(d);
  if (uniformPriorDist != NULL) {
    return o << "UNIFORM" << '\t'
      << uniformPriorDist->GetMinimum() << '\t'
      << uniformPriorDist->GetMaximum();
  } else if (gaussianPriorDist != NULL) {
    return o << "GAUSSIAN" << '\t'
      << gaussianPriorDist->GetMean() << '\t'
      << gaussianPriorDist->GetStandardDeviation();
  } else {
    assert(false);
    return o << "UNKNOWN_PRIOR_TYPE\t0\t1\n";
  }
}
std::ostream & operator <<(std::ostream & o, const madai::Parameter & param) {
  // return o << param.m_Name << '\t'
  //          << param.GetPriorDistribution()->GetPercentile(0.001) << '\t'
  //          << param.GetPriorDistribution()->GetPercentile(0.999) << '\n';
  return o << param.m_Name << '\t' << param.GetPriorDistribution();
}

/**
   The meat of the program.  Interactive Query of the model. */
bool Interact(
    madai::GaussianProcessEmulator & gpme,
    std::istream & input,
    std::ostream & output,
    bool quietFlag)
{
  unsigned int p = gpme.m_NumberParameters;
  unsigned int t = gpme.m_NumberOutputs;
  std::vector< double > the_point(p,0.0);
  std::vector< double > the_mean(t,0.0);
  std::vector< double > the_covariance((t * t),0.0);

  output.precision(17);
  if (! quietFlag) {
    output
      << "VERSION 1\n"
      << "PARAMETERS\n"
      << p << '\n';
    for(unsigned int i = 0; i < p; i++) {
      output << gpme.m_Parameters[i] << '\n';
    }
    output <<"OUTPUTS\n" << t << '\n';

    for(unsigned int i = 0; i < t; i++) {
      output << gpme.m_OutputNames[i] << '\n';
    }
    output << "COVARIANCE\n" << "TRIANGULAR_MATRIX\n"
           << ((t * (t + 1)) / 2) << '\n';
    /*
      For example, a 5x5 symmetric matrix can be represented with
      (5*(5+1)/2)=15 numbers.  If the layout of the matrix is:
          1 2 3 4 5
          2 6 7 8 9
          3 7 a b c
          4 8 b d e
          5 9 c e f
      Then we will serialize it as:
          1 2 3 4 5 6 7 8 9 a b c d e f
      To save time.
    */
    output << "END_OF_HEADER\n";
  }
  output.flush();

  while (input.good()) {
    for(unsigned int i =0; i < p; i++) {
      if (!(input >> the_point[i]))
        return (input.eof());
    }
    if (! gpme.GetEmulatorOutputsAndCovariance (
            the_point, the_mean, the_covariance))
      return false;
    for(unsigned int i =0; i < t; i++) {
      output << the_mean[i] << '\n';
    }
    for(unsigned int i = 0; i < t; i++) {
      for(unsigned int j = (i); j < t; j++) {
        output << the_covariance[(i*t)+j] << '\n';
      }
    }
    output.flush();
  }
  return (input.eof());
}

int main(int argc, char ** argv) {
  std::string StatisticsDirectory;
  if ( argc > 1 ) {
    StatisticsDirectory = std::string( argv[1] );
  } else {
    std::cerr << useage << '\n';
    return EXIT_FAILURE;
  }
  madai::GaussianProcessEmulator gpme;
  struct RuntimeOpts options;
  if ( StatisticsDirectory == "-" ) {
    madai::GaussianProcessEmulatorSingleFileReader singleFileReader;
    singleFileReader.LoadTrainingData( &gpme, std::cin );
  } else {
    madai::EnsurePathSeparatorAtEnd( StatisticsDirectory );
    madai::RuntimeParameterFileReader RPFR;
    RPFR.ParseFile( StatisticsDirectory +
                    madai::Paths::RUNTIME_PARAMETER_FILE );
    char** Args = RPFR.GetArguments();
    int NArgs = RPFR.GetNumberOfArguments();
    if ( !parseRuntimeOptions( NArgs, Args, options ) ) {
      std::cerr << "Error parsing runtime options.\n";
      return EXIT_FAILURE;
    }
    std::string MOD = StatisticsDirectory + options.ModelOutputDirectory;
    std::string ERD = StatisticsDirectory + options.ExperimentalResultsDirectory;
    madai::GaussianProcessEmulatorDirectoryReader directoryReader;
    if ( !directoryReader.LoadTrainingData(&gpme, MOD, StatisticsDirectory, ERD) ) {
      std::cerr << "Error loading training data.\n";
      return EXIT_FAILURE;
    }
    if ( !directoryReader.LoadPCA(&gpme, StatisticsDirectory) ) {
      std::cerr << "Error loading PCA data.\n";
      return EXIT_FAILURE;
    }
    if ( !directoryReader.LoadEmulator(&gpme, StatisticsDirectory) ) {
      std::cerr << "Error loading the emulator state data.\n";
      return EXIT_FAILURE;
    }
  }
  
  if (gpme.m_Status != madai::GaussianProcessEmulator::READY)
    return EXIT_FAILURE;

  if (Interact(gpme, std::cin, std::cout, options.quietFlag))
    return EXIT_SUCCESS;
  else
    return EXIT_FAILURE;
}
