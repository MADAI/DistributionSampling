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
#include <cassert>  // assert
#include <cstdlib>  // std::atof
#include <cmath>    // std::sqrt, std::pow
#include <iostream> // std::cout
#include <fstream>  // std::ifstream
#include <vector>   // std::vector
#include <string>   // std::string
#include <sstream>  // std::stringstream
#include <iomanip>  // std::setw
#include <limits>   // infinity

#include "ApplicationUtilities.h"
#include "GaussianProcessEmulatorDirectoryFormatIO.h"
#include "Paths.h"
#include "System.h"

#include "madaisys/SystemTools.hxx"


int main(int argc, char ** argv) {

  if (argc < 4) {
    std::cerr
      << "Usage\n  " << argv[0]
      << " <statistics directory> <trace file> <number of samples>\n";
    return EXIT_FAILURE;
  }

  std::string statisticsDirectory( argv[1] );
  madai::EnsurePathSeparatorAtEnd( statisticsDirectory );

  std::vector< madai::Parameter > parameters;
  int numberOfParameters = 0;

  if ( ! madai::GaussianProcessEmulatorDirectoryFormatIO::ParseParameters(
           parameters, numberOfParameters, statisticsDirectory, false )) {
    std::cerr
      << "Could not read parameters from prior file '"
      << statisticsDirectory << madai::Paths::PARAMETER_PRIORS_FILE << "'\n";
    return EXIT_FAILURE;
  }
  assert ( numberOfParameters == static_cast<int>( parameters.size() ) );

  std::vector< std::string > outputNames;
  int numberOfOutputs = 0;
  if ( ! madai::GaussianProcessEmulatorDirectoryFormatIO::ParseOutputs(
           outputNames, numberOfOutputs, statisticsDirectory, false ) ) {
    std::cerr
      << "Could not read outputs from file '"
      << statisticsDirectory << madai::Paths::OBSERVABLE_NAMES_FILE << "'\n";
    return EXIT_FAILURE;
  }
  assert( numberOfOutputs == static_cast<int>( outputNames.size() ) );

  std::string traceFile( statisticsDirectory );
  traceFile += madai::Paths::TRACE_DIRECTORY + madai::Paths::SEPARATOR +
    argv[2];
  int numberOfPosteriorSamples = atoi(argv[3]);

  if ( !madai::System::IsFile( traceFile ) ) {
    std::cerr << "Trace file '" << traceFile << "' does not exist or is a directory.\n";
    return EXIT_FAILURE;
  }

  std::ifstream trace(traceFile.c_str());
  if ( !trace.good() ) {
    std::cerr << "Error reading trace file '" << traceFile << "'.\n";
    return EXIT_FAILURE;
  }

  std::string header;
  std::getline(trace, header);
  std::vector<std::string> headers = madai::SplitString(header, ',');

  // Drop the quotes from around the file names
  for ( size_t i = 0; i < headers.size(); ++i ) {
    size_t firstQuote = headers[i].find_first_of( '"' );
    if ( firstQuote != std::string::npos ) {
      size_t secondQuote = headers[i].find_last_of( '"');
      if ( secondQuote != std::string::npos ) {
        headers[i] = headers[i].substr( firstQuote+1, (secondQuote-firstQuote-1) );
      }
    }

    std::cout << headers[i];
    if ( i < headers.size() - 1 ) {
      std::cout << ", ";
    }
  }
  std::cout << '\n';

  size_t numberOfFields = headers.size();
  assert(static_cast<int>(numberOfFields) == numberOfParameters + numberOfOutputs + 1);

  std::string line;
  size_t lineCount = 0, bestIndex = 0;
  std::vector< std::vector< double > > values(numberOfParameters);
  std::vector< std::vector< double > > outputs(numberOfOutputs);

  double bestLogLikelihood = -std::numeric_limits< double >::infinity();

  while (std::getline(trace,line)) {
    std::vector<std::string> fields = madai::SplitString(line, ',');
    assert(numberOfFields == fields.size());

    for (int i = 0; i < numberOfParameters; ++i) {
      double value = std::atof( fields[i].c_str() );
      values[i].push_back(value);
      std::cout << value << ", ";
    }

    for (int i = 0; i < numberOfOutputs; ++i) {
      double value = std::atof( fields[i + numberOfParameters].c_str() );
      outputs[i].push_back(value);
      if ( i < numberOfOutputs - 1 ) {
        std::cout << value << ", ";
      }
    }
    std::cout << "\n";

    double logLikelihood = std::atof( fields[numberOfFields - 1].c_str() );
    if ( logLikelihood > bestLogLikelihood ) {
      bestLogLikelihood = logLikelihood;
      bestIndex = lineCount;
    }
    ++lineCount;
  }
  trace.close();

  std::cout << "Number of rows: " << lineCount << "\n";

  assert(numberOfPosteriorSamples < int(lineCount));
  for ( int isample = 0; isample < numberOfPosteriorSamples; isample++ ) {
    // Make the posterior sample model output directory
    std::stringstream ssPosteriorDirectory;
    ssPosteriorDirectory << statisticsDirectory << "posterior_model_output"
                         << madai::Paths::SEPARATOR << "run" << std::setw(4)
                         << std::setfill('0') << isample
                         << madai::Paths::SEPARATOR;
    std::string posteriorDirectory( ssPosteriorDirectory.str() );
    bool directoryCreated = madaisys::SystemTools::MakeDirectory( posteriorDirectory.c_str() );
    if ( !directoryCreated ) {
      std::cerr << "Could not create directory '" << posteriorDirectory
                << "'.\n";
      return EXIT_FAILURE;
    }

    unsigned int iline = lrint(-1+(isample+1)*lineCount /
                               numberOfPosteriorSamples);
    std::cout << "Sampling row " << iline << "\n";
    assert(iline < lineCount);

    // Write parameters
    std::stringstream ssParameters;
    ssParameters << posteriorDirectory << madai::Paths::SEPARATOR
                 << "parameters.dat";
    std::string filename = ssParameters.str();
    std::ofstream parameterFile( filename.c_str() );
    if ( !parameterFile.is_open() ) {
      std::cerr << "Could not open parameter file '" << filename << "'.\n";
      return EXIT_FAILURE;
    }

    for ( int ipar = 0; ipar < numberOfParameters; ipar++ ) {
      parameterFile << headers[ipar] << " " << values[ipar][iline] << "\n";
    }
    parameterFile.close();

    // Write outputs
    std::stringstream ssOutputs;
    ssOutputs << posteriorDirectory << madai::Paths::SEPARATOR
              << "trace_results.dat";
    filename = ssOutputs.str();
    std::ofstream outputsFile( filename.c_str() );
    if ( !outputsFile.is_open() ) {
      std::cerr << "Could not open outputs file '" << filename << "'.\n";
      return EXIT_FAILURE;
    }

    for ( int ires = 0; ires < numberOfOutputs; ++ires ) {
      outputsFile << headers[numberOfParameters + ires] << " "
                  << outputs[ires][iline] << "\n";
    }
    outputsFile.close();
  }


  return EXIT_SUCCESS;
}
