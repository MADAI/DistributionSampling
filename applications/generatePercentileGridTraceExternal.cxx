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
#include "ExternalModel.h"
#include "RuntimeParameterFileReader.h"
#include "ApplicationUtilities.h"
#include "Paths.h"
#include "Trace.h"

#include "madaisys/SystemTools.hxx"

using madai::Model;
using madai::Paths;

static const int DEFAULT_PERCENTILE_GRID_NUMBER_OF_SAMPLES = 100;


int main(int argc, char ** argv) {

  if (argc < 3) {
    std::cerr << "Usage:\n"
              << "    generatePercentileGridTraceExternal <StatisticsDirectory> <OutputFileName>\n"
              << "\n"
              << "This file generates a sampling of an external model on a\n"
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
              << "EXPERIMENTAL_RESULTS_DIRECTORY <value> (default: "
              << Paths::DEFAULT_EXPERIMENTAL_RESULTS_DIRECTORY << ")\n"
              << "PERCENTILE_GRID_SAMPLES <value> (default: "
              << DEFAULT_PERCENTILE_GRID_NUMBER_OF_SAMPLES << ")\n"
              << "EXTERNAL_MODEL_EXECUTABLE <value> (default: none)\n"
              << "EXTERNAL_MODEL_ARGUMENTS\n"
              << "<Argument1>\n"
              << "<Argument2>\n"
              << "...\n"
              << "<LastAgument>\n"
              << "ARGUMENTS_DONE\n";

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
  std::string experimentalResultsDirectory =
    madai::GetExperimentalResultsDirectory( statisticsDirectory, settings );

  int numberOfSamples = DEFAULT_PERCENTILE_GRID_NUMBER_OF_SAMPLES;
  if ( settings.HasOption( "PERCENTILE_GRID_NUMBER_OF_SAMPLES" ) ) {
    numberOfSamples = atoi( settings.GetOption( "PERCENTILE_GRID_NUMBER_OF_SAMPLES" ).c_str() );
  }

  std::string executable;
  if ( settings.HasOption( "EXTERNAL_MODEL_EXECUTABLE" ) ) {
    executable = settings.GetOption( "EXTERNAL_MODEL_EXECUTABLE" );
  }

  // Split arguments into vector of strings
  std::vector< std::string > arguments;
  if ( settings.HasOption( "EXTERNAL_MODEL_ARGUMENTS" ) ) {
    std::string argumentsString = settings.GetOption( "EXTERNAL_MODEL_ARGUMENTS" );
    arguments = madai::SplitString( argumentsString, ' ' );
  }

  madai::ExternalModel em;
  em.StartProcess( executable, arguments );
  if (! em.IsReady()) {
    std::cerr << "Something is wrong with the external model\n";
    return EXIT_FAILURE;
  }

  em.SetUseModelCovarianceToCalulateLogLikelihood( false );

  std::string observationsFile = experimentalResultsDirectory +
    madai::Paths::SEPARATOR + madai::Paths::RESULTS_FILE;
  std::ifstream observations( observationsFile.c_str() );
  if ( madai::Model::NO_ERROR != em.LoadObservations( observations ) ) {
    std::cerr << "Error loading observations.\n";
    em.StopProcess();

    return EXIT_FAILURE;
  }
  observations.close();

  madai::PercentileGridSampler sampler;
  sampler.SetModel( &em );
  sampler.SetNumberSamples( numberOfSamples );
  numberOfSamples = sampler.GetNumberSamples();

  madai::Trace trace;

  int step = numberOfSamples / 100, percent = 0;
  for (int count = 0; count < numberOfSamples; count ++) {
    if (count % step == 0)
      std::cout <<  '\r' << percent++ << "%";
    trace.Add(sampler.NextSample());
  }
  std::cout << "\r" ;

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
                        em.GetParameters(),
                        em.GetScalarOutputNames() );

  std::cout << "Wrote percentile grid trace to file '" << outputFilePath << "'.\n";

  return EXIT_SUCCESS;
}
