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
#include <iomanip>  // std::setw
#include <iostream> // std::cout
#include <fstream>  // std::ifstream
#include <limits>   // infinity
#include <sstream>  // std::stringstream
#include <string>   // std::string
#include <vector>   // std::vector

#include "ApplicationUtilities.h"
#include "Defaults.h"
#include "GaussianProcessEmulatorDirectoryFormatIO.h"
#include "Paths.h"
#include "RuntimeParameterFileReader.h"
#include "System.h"

#include "madaisys/SystemTools.hxx"


int main(int argc, char ** argv) {

  if (argc < 4) {
    std::cerr
      << "Usage\n  " << argv[0]
      << " <statistics directory> <trace file> <number of samples>\n"
      << "\n"
      << "This program samples a trace file located at \n"
      << "<statistics directory>/trace/<trace file>. The argument \n"
      << "<number of samples> specifies how many samples are taken from the \n"
      << "trace. Each sample is stored in a directory specified by the \n"
      << "setting POSTERIOR_ANALYSIS_DIRECTORY in the same format as the \n"
      << "model output generated by madai_generate_training_points. In \n"
      << "addition, a file named 'trace_results.dat' is stored in each \n"
      << "run directory. This file contains the observables listed in the \n"
      << "trace.\n"
      << "\n"
      << "The output of this program can be used to generate results from the \n"
      << "actual model to compare against the output from the emulator. If \n"
      << "your original trace is from an actual model, then use of this \n"
      << "program is not necessarily useful.\n"
      << "\n"
      << "Format of entries in " << madai::Paths::RUNTIME_PARAMETER_FILE
      << ":\n\n"
      << "POSTERIOR_ANALYSIS_DIRECTORY <value> (default: "
      << madai::Defaults::POSTERIOR_ANALYSIS_DIRECTORY << ")\n"
      << "VERBOSE <value> (default: "
      << madai::Defaults::VERBOSE << ")\n";

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

  std::vector< std::string > observableNames;
  int numberOfObservables = 0;
  if ( ! madai::GaussianProcessEmulatorDirectoryFormatIO::ParseOutputs(
           observableNames, numberOfObservables, statisticsDirectory, false ) ) {
    std::cerr
      << "Could not read observable names from file '"
      << statisticsDirectory << madai::Paths::OBSERVABLE_NAMES_FILE << "'\n";
    return EXIT_FAILURE;
  }
  assert( numberOfObservables == static_cast<int>( observableNames.size() ) );

  std::string traceFile( statisticsDirectory );
  traceFile += madai::Paths::TRACE_DIRECTORY + madai::Paths::SEPARATOR +
    argv[2];
  int numberOfPosteriorSamples = atoi(argv[3]);

  if ( !madai::System::IsFile( traceFile ) ) {
    std::cerr << "Trace file '" << traceFile << "' does not exist or is a directory.\n";
    return EXIT_FAILURE;
  }

  std::ifstream trace( traceFile.c_str() );
  if ( !trace.good() ) {
    std::cerr << "Error reading trace file '" << traceFile << "'.\n";
    return EXIT_FAILURE;
  }

  std::string header;
  std::getline( trace, header );
  std::vector<std::string> headers = madai::SplitString( header, ',' );

  // Drop the quotes from around the file names
  for ( size_t i = 0; i < headers.size(); ++i ) {
    size_t firstQuote = headers[i].find_first_of( '"' );
    if ( firstQuote != std::string::npos ) {
      size_t secondQuote = headers[i].find_last_of( '"');
      if ( secondQuote != std::string::npos ) {
        headers[i] = headers[i].substr( firstQuote+1, (secondQuote-firstQuote-1) );
      }
    }
  }

  size_t numberOfFields = headers.size();
  assert(static_cast<int>(numberOfFields) == numberOfParameters + numberOfObservables + 1);

  std::string line;
  size_t lineCount = 0, bestIndex = 0;
  std::vector< std::vector< double > > values(numberOfParameters);
  std::vector< std::vector< double > > observables(numberOfObservables);

  double bestLogLikelihood = -std::numeric_limits< double >::infinity();

  while ( std::getline( trace, line ) ) {
    std::vector<std::string> fields = madai::SplitString( line, ',' );
    assert( numberOfFields == fields.size() );

    for ( int i = 0; i < numberOfParameters; ++i ) {
      double value = std::atof( fields[i].c_str() );
      values[i].push_back( value );
    }

    for ( int i = 0; i < numberOfObservables; ++i ) {
      double value = std::atof( fields[i + numberOfParameters].c_str() );
      observables[i].push_back( value );
    }

    double logLikelihood = std::atof( fields[numberOfFields - 1].c_str() );
    if ( logLikelihood > bestLogLikelihood ) {
      bestLogLikelihood = logLikelihood;
      bestIndex = lineCount;
    }
    ++lineCount;
  }
  trace.close();

  // Make the requested directory
  std::string posteriorAnalysisDirectory =
    madai::GetPosteriorAnalysisDirectory( statisticsDirectory, settings );
  if ( !madaisys::SystemTools::MakeDirectory( posteriorAnalysisDirectory.c_str() ) ) {
    std::cerr << "Could not create posterior analysis directory '"
              << posteriorAnalysisDirectory << "'.\n";
    return EXIT_FAILURE;
  }

  assert( numberOfPosteriorSamples < int(lineCount) );
  for ( int isample = 0; isample < numberOfPosteriorSamples; isample++ ) {
    // Make the posterior sample model output directory
    std::stringstream ssRunDirectory;
    ssRunDirectory << posteriorAnalysisDirectory
                   << madai::Paths::SEPARATOR << "run" << std::setw(4)
                   << std::setfill('0') << isample
                   << madai::Paths::SEPARATOR;
    std::string runDirectory( ssRunDirectory.str() );
    bool directoryCreated = madaisys::SystemTools::MakeDirectory( runDirectory.c_str() );
    if ( !directoryCreated ) {
      std::cerr << "Could not create directory '" << posteriorAnalysisDirectory
                << "'.\n";
      return EXIT_FAILURE;
    }

    unsigned int iline = lrint(-1+(isample+1)*lineCount /
                               numberOfPosteriorSamples);
    assert(iline < lineCount);

    // Write parameters
    std::string parameterFileName =
      runDirectory + madai::Paths::SEPARATOR + "parameters.dat";
    std::ofstream parameterFile( parameterFileName.c_str() );
    if ( !parameterFile.is_open() ) {
      std::cerr << "Could not open parameter file '" << parameterFileName << "'.\n";
      return EXIT_FAILURE;
    }

    for ( int i = 0; i < numberOfParameters; i++ ) {
      parameterFile << headers[i] << " " << values[i][iline] << "\n";
    }
    parameterFile.close();

    // Write observables
    std::string observablesFileName =
      runDirectory + madai::Paths::SEPARATOR + "trace_results.dat";
    std::ofstream observablesFile( observablesFileName.c_str() );
    if ( !observablesFile.is_open() ) {
      std::cerr << "Could not open outputs file '" << observablesFileName << "'.\n";
      return EXIT_FAILURE;
    }

    for ( int i = 0; i < numberOfObservables; ++i ) {
      observablesFile << headers[numberOfParameters + i] << " "
                  << observables[i][iline] << "\n";
    }
    observablesFile.close();
  }


  return EXIT_SUCCESS;
}
