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
#include "GaussianProcessEmulatorDirectoryFormatIO.h"
#include "Paths.h"
#include "System.h"

int main(int argc, char ** argv) {
  if (argc < 3) {
    std::cerr
      << "Usage\n  " << argv[0]
      << " statistics_directory trace_file\n\n";
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
  assert (numberOfParameters = parameters.size());

  std::vector< std::string > outputNames;
  int numberOfOutputs = 0;

  if ( ! madai::GaussianProcessEmulatorDirectoryFormatIO::ParseOutputs(
           outputNames, numberOfOutputs, statisticsDirectory, false )) {
    std::cerr
      << "Could not read outputs from prior file '"
      << statisticsDirectory << madai::Paths::PARAMETER_PRIORS_FILE << "'\n";
    return EXIT_FAILURE;
  }
  assert (numberOfOutputs = outputNames.size());

  std::string traceFile( statisticsDirectory );
  traceFile.append( argv[2] );

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

  int numberOfFields = headers.size();
  bool gradientsPresent = false;
  if( numberOfFields == numberOfParameters + 3*numberOfOutputs + 1 ) {
    numberOfFields -= 2*numberOfOutputs;
    gradientsPresent = true;
  }
  assert(numberOfFields == numberOfParameters + numberOfOutputs + 1);

  std::string line;
  size_t lineCount = 0, bestIndex = 0;
  std::vector< double > sums(numberOfFields - 1, 0.0);
  std::vector< std::vector< double > > values(numberOfFields - 1);

  double bestLogLikelihood = -std::numeric_limits< double >::infinity();

  while (std::getline(trace,line)) {
    std::vector<std::string> fields = madai::SplitString(line, ',');
    assert(numberOfFields == (int) fields.size() - (gradientsPresent?2*numberOfOutputs:0));
    for (int i = 0; i < numberOfFields - 1; ++i) {
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
  std::vector< double > means(numberOfFields - 1,0.0);

  std::vector< double > priorStdDev(numberOfFields - 1,0.0);
  for (int i = 0; i < numberOfParameters; ++i) {
    priorStdDev[i] =
      parameters[i].GetPriorDistribution()->GetStandardDeviation();
  }
  std::cout << std::setw(14) << "parameter";
  std::cout << std::setw(14) << "mean";
  std::cout << std::setw(14) << "std.dev.";
  std::cout << std::setw(14) << "scaled dev.";
  std::cout << std::setw(14) << "best value";
  std::cout << '\n';

  for (int i = 0; i < numberOfFields - 1; ++i) {
    means[i] = sums[i] / lineCount;
  }
  for (int i = 0; i < numberOfParameters; ++i) {
    double variance = 0.0;
    for (size_t k = 0; k < lineCount; ++k) {
      variance += std::pow(values[i][k] - means[i], 2);
    }
    variance /= lineCount;
    std::cout
      << std::setw(14) << parameters[i].m_Name
      << std::setw(14) << means[i]
      << std::setw(14) << std::sqrt(variance)
      << std::setw(14) << std::sqrt(variance) / priorStdDev[i]
      << std::setw(14) << values[i][bestIndex]
      << '\n';
  }

  // Print the relative log-likelihood from the best point
  std::cout << "\nbest log likelihood\n";
  std::cout << std::setw(14) << bestLogLikelihood << "\n";

  std::vector< std::vector< double > > covariancematrix;
  for (int i = 0; i < numberOfFields - 1; ++i)
    covariancematrix.push_back(std::vector< double >(numberOfFields - 1, 0.0));

  for (int i = 0; i < numberOfFields - 1; ++i) {
    for (int j = 0; j <= i; ++j) {
      double covariance = 0.0;
      for (size_t k = 0; k < lineCount; ++k) {
        covariance += (values[i][k] - means[i]) * (values[j][k] - means[j]);
      }
      covariancematrix[i][j] = covariance /= lineCount;
      if (i != j)
        covariancematrix[j][i] = covariancematrix[i][j];
    }
  }

  std::cout << "\ncovariance:\n";
  std::cout << std::setw(14) << "";
  for (int j = 0; j < numberOfParameters; ++j)
    std::cout << std::setw(14) << parameters[j].m_Name;
  std::cout << "\n";
  for (int i = 0; i < numberOfParameters; ++i) {
    std::cout << std::setw(14) << parameters[i].m_Name;
    for (int j = 0; j < numberOfParameters; ++j) {
      std::cout << std::setw(14) << covariancematrix[i][j];
    }
    std::cout << "\n";
  }

  std::cout << "\nscaled covariance:\n";
  std::cout << std::setw(14) << "";
  for (int j = 0; j < numberOfParameters; ++j)
    std::cout << std::setw(14) << parameters[j].m_Name;
  std::cout << "\n";
  for (int i = 0; i < numberOfParameters; ++i) {
    std::cout << std::setw(14) << parameters[i].m_Name;
    for (int j = 0; j < numberOfParameters; ++j) {
      std::cout << std::setw(14) << covariancematrix[i][j] / (
          priorStdDev[i] * priorStdDev[j]);
    }
    std::cout << "\n";
  }

  std::cout << "\nobservable-parameter correlation:\n";
  std::cout << std::setw(14) << "";
  for (int j = 0; j < numberOfParameters; ++j)
    std::cout << std::setw(14) << parameters[j].m_Name;
  std::cout << "\n";
  for (int i = 0; i < numberOfOutputs; ++i) {
    std::cout << std::setw(14) << outputNames[i];
    for (int j = 0; j < numberOfParameters; ++j) {
      std::cout << std::setw(14) << covariancematrix[numberOfParameters + i][j] / (
          std::sqrt(covariancematrix[numberOfParameters + i][numberOfParameters + i]*
          covariancematrix[j][j]));
    }
    std::cout << "\n";
  }


  return EXIT_SUCCESS;
}
