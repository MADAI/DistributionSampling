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

#include <getopt.h>
#include <iostream>
#include <fstream>
#include <cassert>
#include <cstdlib>
#include "GaussianProcessModelEmulator.h"
#include "UniformDistribution.h"
#include "GaussianDistribution.h"

static const char useage [] =
  "useage:\n"
  "  emulate [options] MODEL_SNAPSHOT_FILE.dat\n"
  "\n"
  "Options:\n"
  "\n"
  "  -q\n"
  "  --quiet   Do not print a header before going into query mode.\n"
  "\n"
  "  -h -?     print this dialogue\n"
  "\n";

struct cmdLineOpts{
  bool quietFlag;
  const char * inputFile; /* first non-flag argument  */
};

/**
   Option parsing using getoptlong.  If it fails, returns false. */
bool parseCommandLineOptions(int argc, char** argv, struct cmdLineOpts & opts)
{
  /* note: flags followed with a colon come with an argument */
  static const char optString[] = "qh?";

  // should add a variance option for the pca_decomp
  // and a flag to do return output in pca space
  static const struct option longOpts[] = {
    { "quiet", no_argument , NULL, 'q'},
    { "help", no_argument , NULL, 'h'},
    { NULL, no_argument, NULL, 0}
  };

  // init with default values
  opts.quietFlag = false;
  opts.inputFile = NULL; // default to NULL
  int longIndex, opt;
  opt = getopt_long( argc, argv, optString, longOpts, &longIndex );
  while( opt != -1 ) {
    switch( opt ) {
    case 'q':
      opts.quietFlag = true;
      break;
    case 'h':
      /* fall-through is intentional */
    case '?':
      std::cerr << useage << '\n';
      return false;
      break;
    default:
      /* You won't actually get here. */
      assert(false && "1152677029");
      break;
    }
    opt = getopt_long( argc, argv, optString, longOpts, &longIndex );
  }

  // set the remaining field
  if ((argc - optind) >= 1) {
    opts.inputFile = argv[optind];
  }
  if (opts.inputFile == NULL) {
    std::cerr << useage << '\n';
    return false;
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
    madai::GaussianProcessModelEmulator & gpme,
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
  struct cmdLineOpts options;
  if (!parseCommandLineOptions(argc, argv, options))
    return EXIT_FAILURE;
  std::string inputFile(options.inputFile);
  madai::GaussianProcessModelEmulator gpme;
  if (inputFile == "-")
    /*
      Please note: if you use stdin to feed in the model, you should
      do it like this:
      $ cat model_snapshot_file.dat query_points.dat | emulate
    */
    gpme.Load(std::cin);
  else {
    std::ifstream is (options.inputFile);
    if (is.good()) {
      gpme.Load(is);
    } else {
      std::cerr << "Error opening file " << options.inputFile << '\n';
      return EXIT_FAILURE;
    }
  }
  if (gpme.m_Status != madai::GaussianProcessModelEmulator::READY)
    return EXIT_FAILURE;

  if (Interact(gpme, std::cin, std::cout, options.quietFlag))
    return EXIT_SUCCESS;
  else
    return EXIT_FAILURE;
}
