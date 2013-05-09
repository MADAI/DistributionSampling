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
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

#include "ApplicationUtilities.h"
#include "ExternalModel.h"
#include "MetropolisHastingsSampler.h"
#include "Paths.h"
#include "RuntimeParameterFileReader.h"
#include "Trace.h"
#include "SamplerCSVWriter.h"

#include "madaisys/SystemTools.hxx"

using madai::Paths;

static const int    DEFAULT_MCMC_NUMBER_OF_SAMPLES         = 100;
static const int    DEFAULT_MCMC_NUMBER_OF_BURN_IN_SAMPLES = 0;
static const bool   DEFAULT_MCMC_USE_MODEL_ERROR           = false;
static const double DEFAULT_MCMC_STEP_SIZE                 = 0.1;


int main(int argc, char ** argv) {

  if ( argc < 3 ) {
    std::cerr << "Usage:\n"
              << "    generateMCMCtraceExternal <StatisticsDirectory> <OutputFileName>\n"
              << "\n"
              << "This program produces a Markov Chain Monte Carlo trace from \n"
              << "by evaluating an model defined in an external process. \n"
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
              << "MCMC_NUMBER_OF_SAMPLES <value> (default: "
              << DEFAULT_MCMC_NUMBER_OF_SAMPLES << ")\n"
              << "MCMC_NUMBER_OF_BURN_IN_SAMPLES <value> (default: "
              << DEFAULT_MCMC_NUMBER_OF_BURN_IN_SAMPLES << ")\n"
              << "MCMC_USE_MODEL_ERROR <value> (default: "
              << DEFAULT_MCMC_USE_MODEL_ERROR << ")\n"
              << "MCMC_STEP_SIZE <value> (default: "
              << DEFAULT_MCMC_STEP_SIZE << ")\n"
              << "EXTERNAL_MODEL_EXECUTABLE <value> (default: none)\n"
              << "EXTERNAL_MODEL_ARGUMENTS <Argument1> <Argument2> ... <LastArgument>\n"
              << "VERBOSE <value> (default: false)\n";

    return EXIT_FAILURE;
  }

  std::string statisticsDirectory( argv[1] );
  madai::EnsurePathSeparatorAtEnd( statisticsDirectory );

  madai::RuntimeParameterFileReader settings;
  std::string settingsFile =
    statisticsDirectory + madai::Paths::RUNTIME_PARAMETER_FILE;
  if ( !settings.ParseFile( settingsFile ) ) {
    std::cerr
      << "Could not open runtime parameter file '" << settingsFile << "'\n";
    return EXIT_FAILURE;
  }

  std::string experimentalResultsDirectory =
    madai::GetExperimentalResultsDirectory( statisticsDirectory, settings );

  int numberOfSamples = DEFAULT_MCMC_NUMBER_OF_SAMPLES;
  if ( settings.HasOption( "MCMC_NUMBER_OF_SAMPLES" ) ) {
    numberOfSamples =
      atoi( settings.GetOption( "MCMC_NUMBER_OF_SAMPLES" ).c_str() );
  }
  int numberOfBurnInSamples = DEFAULT_MCMC_NUMBER_OF_BURN_IN_SAMPLES;
  if ( settings.HasOption( "MCMC_NUMBER_OF_BURN_IN_SAMPLES" ) ) {
    numberOfBurnInSamples =
      atoi( settings.GetOption( "MCMC_NUMBER_OF_BURN_IN_SAMPLES" ).c_str() );
  }
  bool useModelError = DEFAULT_MCMC_USE_MODEL_ERROR;
  if ( settings.HasOption( "MCMC_USE_MODEL_ERROR" ) ) {
    useModelError = ( settings.GetOption( "MCMC_USE_MODEL_ERROR" ) == "true" );
  }
  double stepSize = DEFAULT_MCMC_STEP_SIZE;
  if ( settings.HasOption( "MCMC_STEP_SIZE" ) ) {
    stepSize = atof( settings.GetOption( "MCMC_STEP_SIZE" ).c_str() );
  }
  std::string executable;
  if ( settings.HasOption( "EXTERNAL_MODEL_EXECUTABLE" ) ) {
    executable = settings.GetOption( "EXTERNAL_MODEL_EXECUTABLE" );
  }

  // Split arguments into vector of strings
  std::vector< std::string > arguments;
  if ( settings.HasOption( "EXTERNAL_MODEL_ARGUMENTS" ) ) {
    std::string argumentsString =
      settings.GetOption( "EXTERNAL_MODEL_ARGUMENTS" );
    arguments = madai::SplitString( argumentsString, ' ' );
  }

  madai::ExternalModel em;
  em.StartProcess( executable, arguments );
  if (! em.IsReady()) {
    std::cerr << "Something is wrong with the external model\n";
    return EXIT_FAILURE;
  }

  std::string observationsFile = experimentalResultsDirectory +
    madai::Paths::SEPARATOR + madai::Paths::RESULTS_FILE;
  std::ifstream observations( observationsFile.c_str() );
  if ( madai::Model::NO_ERROR != em.LoadObservations( observations ) ) {
    std::cerr << "Error loading observations.\n";
    em.StopProcess();

    return EXIT_FAILURE;
  }
  observations.close();

  madai::MetropolisHastingsSampler mcmc;
  mcmc.SetStepSize( stepSize );

  std::string traceDirectory =
    statisticsDirectory + madai::Paths::TRACE_DIRECTORY;
  madaisys::SystemTools::MakeDirectory( traceDirectory.c_str() );
  std::string outputFileName( argv[2] );
  std::string outputFilePath =
    traceDirectory + madai::Paths::SEPARATOR + outputFileName;

  std::ofstream outFile(outputFilePath.c_str());
  if ( !outFile.good() ) {
    std::cerr << "Could not open trace file '" << outputFilePath << "' for writing.\n";
    return EXIT_FAILURE;
  }

  int returnCode = madai::SamplerCSVWriter::GenerateSamplesAndSaveToFile(
      &mcmc,
      &em,
      outFile,
      numberOfSamples,
      numberOfBurnInSamples,
      useModelError,
      &(std::cerr));

  if ( settings.GetOptionAsBool( "VERBOSE", false ) ) {
    if ( returnCode == EXIT_SUCCESS ) {
      std::cout << "Succeeded writing trace file '" << outputFilePath << "'.\n";
    } else {
      std::cerr << "Could not write trace file '" << outputFilePath << "'.\n";
    }
  }

  return returnCode;
}
