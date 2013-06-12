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

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include "PercentileGridSampler.h"
#include "GaussianProcessEmulatedModel.h"
#include "RuntimeParameterFileReader.h"
#include "ApplicationUtilities.h"
#include "Paths.h"
#include "Defaults.h"
#include "Trace.h"

#include "madaisys/SystemTools.hxx"

using madai::Model;
using madai::Paths;

int main(int argc, char ** argv) {

  if (argc < 3) {
    std::cerr
      << "Usage:\n"
      << "    " << argv[0] << " <StatisticsDirectory> <OutputFileName>\n"
      << "\n"
      << "This file generates a sampling of an emulated model on a\n"
      << "regular lattice of sample points.\n"
      << "\n"
      << "<StatisticsDirectory> is the directory in which all \n"
      << "statistics data are stored. It contains the parameter file "
      << Paths::RUNTIME_PARAMETER_FILE << "\n"
      << "\n"
      << "<OutputFileName> is the name of the comma-separated value-format \n"
      << "file in which the trace will be written. This file will be \n"
      << "written in the directory <StatisticsDirectory>/trace/.\n"
      << "\n"
      << "Format of entries in " << Paths::RUNTIME_PARAMETER_FILE
      << ":\n\n"
      << "MODEL_OUTPUT_DIRECTORY <value> (default: "
      << madai::Defaults::MODEL_OUTPUT_DIRECTORY << ")\n"
      << "EXPERIMENTAL_RESULTS_FILE <value> (default: "
      << madai::Defaults::EXPERIMENTAL_RESULTS_FILE << ")\n"
      << "PERCENTILE_GRID_NUMBER_OF_SAMPLES <value> (default: "
      << madai::Defaults::PERCENTILE_GRID_NUMBER_OF_SAMPLES << ")\n"
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

  std::string modelOutputDirectory =
    madai::GetModelOutputDirectory( statisticsDirectory, settings );
  std::string experimentalResultsFile =
    madai::GetExperimentalResultsFile( statisticsDirectory, settings );

  int numberOfSamples = settings.GetOptionAsInt(
      "PERCENTILE_GRID_NUMBER_OF_SAMPLES",
      madai::Defaults::PERCENTILE_GRID_NUMBER_OF_SAMPLES);

  madai::GaussianProcessEmulatedModel gpem;
  if ( gpem.LoadConfiguration( statisticsDirectory,
                               modelOutputDirectory,
                               experimentalResultsFile ) != madai::Model::NO_ERROR ) {
    std::cerr << "Error in GaussianProcessEmulatedModel::LoadConfiguration\n";
    return EXIT_FAILURE;
  }

  gpem.SetUseModelCovarianceToCalulateLogLikelihood( false );

  std::string observationsFile = experimentalResultsFile + Paths::SEPARATOR +
    Paths::RESULTS_FILE;
  std::ifstream observations( observationsFile.c_str() );
  if ( madai::Model::NO_ERROR != LoadObservations( &gpem, observations ) ) {
    std::cerr << "Error loading observations.\n";
    return EXIT_FAILURE;
  }
  observations.close();

  madai::PercentileGridSampler sampler;
  sampler.SetModel( &gpem );
  sampler.SetNumberOfSamples( numberOfSamples );
  numberOfSamples = sampler.GetNumberOfSamples();

  madai::Trace trace;

  int step = numberOfSamples / 100, percent = 0;
  for (int count = 0; count < numberOfSamples; count ++) {
    if (count % step == 0)
      std::cerr <<  '\r' << percent++ << "%";
    trace.Add(sampler.NextSample());
  }
  std::cerr << "\r" ;

  std::string traceDirectory = statisticsDirectory + madai::Paths::TRACE_DIRECTORY;
  madaisys::SystemTools::MakeDirectory( traceDirectory.c_str() );
  std::string outputFileName( argv[2] );
  std::string outputFilePath = traceDirectory + Paths::SEPARATOR + outputFileName;
  std::ofstream out( outputFilePath.c_str() );
  if ( !out.good() ) {
    std::cerr << "Could not open output file '" << outputFilePath << "' for writing.\n";
    return EXIT_FAILURE;
  }

  trace.WriteCSVOutput( out,
                        gpem.GetParameters(),
                        gpem.GetScalarOutputNames() );

  if ( settings.GetOptionAsBool( "VERBOSE", madai::Defaults::VERBOSE ) ) {
    std::cout << "Wrote percentile grid trace to file '" << outputFilePath << "'.\n";
  }

  return EXIT_SUCCESS;
}
