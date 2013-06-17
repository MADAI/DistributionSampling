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
#include <cassert> // assert
#include <cstdlib> // std::atof
#include <cmath> // std::sqrt, std::pow
#include <iostream> // std::cout
#include <fstream> // std::ifstream
#include <vector> // std::vector
#include <string> // std::string
#include <sstream> // std::stringstream
#include <iomanip> // std::setw
#include <limits> // inifity

#include "ApplicationUtilities.h"
#include "GaussianProcessEmulatorDirectoryReader.h"
#include "Paths.h"
#include "System.h"

int main(int argc, char ** argv) {
  if (argc < 4) {
    std::cerr
      << "Usage\n  " << argv[0]
      << " statistics_directory trace_file N_samples\n\n";
    return EXIT_FAILURE;
  }
  std::string statisticsDirectory( argv[1] );
  madai::EnsurePathSeparatorAtEnd( statisticsDirectory );

  std::vector< madai::Parameter > parameters;
  int numberOfParameters = 0;

  if ( ! madai::GaussianProcessEmulatorDirectoryReader::ParseParameters(
           parameters, numberOfParameters, statisticsDirectory, false )) {
    std::cerr
      << "Could not read parameters from prior file '"
      << statisticsDirectory << madai::Paths::PARAMETER_PRIORS_FILE << "'\n";
    return EXIT_FAILURE;
  }
  assert (numberOfParameters = parameters.size());

  std::string traceFile( statisticsDirectory );
  traceFile.append( madai::Paths::TRACE_DIRECTORY );
  traceFile.append( "/" );
  traceFile.append( argv[2] );
  int N_samples=atoi(argv[3]);

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
  std::getline(trace,header);
  std::vector<std::string> headers = madai::SplitString(header, ',');

  size_t numberOfFields = headers.size();
  assert(static_cast<int>(numberOfFields) > numberOfParameters);

  std::string line;
  size_t lineCount = 0, bestIndex = 0;
  std::vector< double > sums(numberOfParameters, 0.0);
  std::vector< std::vector< double > > values(numberOfParameters);

  double bestLogLikelihood = -std::numeric_limits< double >::infinity();

  while (std::getline(trace,line)) {
    std::vector<std::string> fields = madai::SplitString(line, ',');
    assert(numberOfFields == fields.size());
    for (int i = 0; i < numberOfParameters; ++i) {
      double value = std::atof(fields[i].c_str());
      values[i].push_back(value);
      sums[i] += value;
    }
    double logLikelihood = std::atof(fields[numberOfFields - 1].c_str());
    if (logLikelihood > bestLogLikelihood) {
      bestLogLikelihood = logLikelihood;
      bestIndex = lineCount;
    }
    ++lineCount;
  }
  trace.close();

  int iline,ipar;
  assert(N_samples > int(lineCount));
  std::ofstream parfile;
  std::string filename;
  std::stringstream sst;
  for(int isample=0;isample<N_samples;isample++){
    sst << "posterior_model_output/run" << isample << "parameters.dat";
    filename=sst.str();
    parfile.open(filename.c_str());
    iline=lrint(-1+(isample+1)*lineCount/N_samples);
    assert(iline < lineCount);
    for(ipar=0;ipar<numberOfParameters;ipar++){
      parfile << header[ipar] << values[iline][ipar] << std::endl;
    }
    parfile.close();
  }


  return EXIT_SUCCESS;
}
