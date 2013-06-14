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

/**
ACKNOWLEDGMENTS:
  This software was written in 2012-2013 by Hal Canary
  <cs.unc.edu/~hal>, based off of the MADAIEmulator program (Copyright
  2009-2012 Duke University) by C.Coleman-Smith <cec24@phy.duke.edu>
  in 2010-2012 while working for the MADAI project <http://madai.us/>.
*/

#include <iostream>
#include <fstream>

#include "ApplicationUtilities.h"
#include "GaussianProcessEmulator.h"
#include "GaussianDistribution.h"
#include "GaussianProcessEmulatorDirectoryFormatIO.h"
#include "RuntimeParameterFileReader.h"
#include "UniformDistribution.h"
#include "Paths.h"
#include "Defaults.h"


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
  return o << param.m_Name << '\t' << param.GetPriorDistribution();
}

/**
   The meat of the program.  Interactive query of the model. */
bool Interact(
    madai::GaussianProcessEmulator & gpme,
    std::istream & input,
    std::ostream & output,
    bool writeHeader )
{
  unsigned int p = gpme.m_NumberParameters;
  unsigned int t = gpme.m_NumberOutputs;
  std::vector< double > the_point(p,0.0);
  std::vector< double > the_mean(t,0.0);
  std::vector< double > the_covariance((t * t),0.0);

  output.precision(17);
  if ( writeHeader ) {
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
  if ( argc < 2 ) {
    std::cerr << "Usage:\n"
              << "    " << argv[0] << " <StatisticsDirectory>\n"
              << "\n"
              << "This program provides a pipe interface to a trained \n"
              << "emulator. \n"
              << "\n"
              << "<StatisticsDirectory> is the directory in which all \n"
              << "statistics data are stored. It contains the parameter file "
              << madai::Paths::RUNTIME_PARAMETER_FILE << "\n"
              << "\n"
              << "Format of entries in " << madai::Paths::RUNTIME_PARAMETER_FILE
              << ":\n\n"
              << "MODEL_OUTPUT_DIRECTORY <value> (default: "
              << madai::Defaults::MODEL_OUTPUT_DIRECTORY << ")\n"
              << "EXPERIMENTAL_RESULTS_FILE <value> (default: "
              << madai::Defaults::EXPERIMENTAL_RESULTS_FILE << ")\n"
              << "EMULATE_QUIET <value> ("
              << madai::Defaults::EMULATE_QUIET << ")\n"
              << "READER_VERBOSE <value> (default: "
              << madai::Defaults::READER_VERBOSE << ")\n";

    return EXIT_FAILURE;
  }
  std::string statisticsDirectory( argv[1] );
  madai::EnsurePathSeparatorAtEnd( statisticsDirectory );

  madai::RuntimeParameterFileReader settings;
  std::string settingsFile = statisticsDirectory + madai::Paths::RUNTIME_PARAMETER_FILE;
  if ( !settings.ParseFile( settingsFile ) ) {
    std::cerr << "Could not open runtime parameter file '" << settingsFile << "'\n";
    return EXIT_FAILURE;
  }

  std::string modelOutputDirectory =
    madai::GetModelOutputDirectory( statisticsDirectory, settings );
  std::string experimentalResultsFile =
    madai::GetExperimentalResultsFile( statisticsDirectory, settings );

  bool emulatorWriteHeader = settings.GetOptionAsBool(
      "EMULATE_WRITE_HEADER",
      madai::Defaults::EMULATE_WRITE_HEADER);

  madai::GaussianProcessEmulator gpe;
  madai::GaussianProcessEmulatorDirectoryFormatIO directoryReader;
  bool verbose = settings.GetOptionAsBool(
      "READER_VERBOSE", madai::Defaults::READER_VERBOSE );
  directoryReader.SetVerbose( verbose );

  if ( !directoryReader.LoadTrainingData( &gpe,
                                          modelOutputDirectory,
                                          statisticsDirectory,
                                          experimentalResultsFile ) ) {
    std::cerr << "Error loading training data.\n";
    return EXIT_FAILURE;
  }

  if ( !directoryReader.LoadPCA( &gpe, statisticsDirectory ) ) {
    std::cerr << "Error loading PCA data.\n";
    return EXIT_FAILURE;
  }

  if ( !directoryReader.LoadEmulator( &gpe, statisticsDirectory ) ) {
    std::cerr << "Error loading the emulator state data.\n";
    return EXIT_FAILURE;
  }

  if ( gpe.GetStatus() != madai::GaussianProcessEmulator::READY ) {
    return EXIT_FAILURE;
  }

  if ( Interact( gpe, std::cin, std::cout, emulatorWriteHeader ) ) {
    return EXIT_SUCCESS;
  } else {
    return EXIT_FAILURE;
  }
}
