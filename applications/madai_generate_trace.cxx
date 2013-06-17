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
#include "Defaults.h"
#include "ExternalModel.h"
#include "MetropolisHastingsSampler.h"
#include "GaussianProcessEmulatedModel.h"
#include "RuntimeParameterFileReader.h"
#include "Paths.h"
#include "PercentileGridSampler.h"
#include "Trace.h"
#include "SamplerCSVWriter.h"

#include "madaisys/SystemTools.hxx"


int main(int argc, char ** argv) {

  if (argc < 3) {
    std::cerr
      << "Usage:\n"
      << "    " << argv[0] << " <StatisticsDirectory> <OutputFileName>\n"
      << "\n"
      << "This program produces a Markov Chain Monte Carlo trace by either \n"
      << "evaluating a model defined in an external process or evaluating a \n"
      << "trained emulator. The program madai_pca_decompose must have been \n"
      << "run on <StatisticsDirectory> prior to running this program and if \n"
      << "no EXTERNAL_MODEL_EXECUTABLE is specified in the settings file, \n"
      << "madai_train_emulator must have been run as well.\n"
      << "\n"
      << "<StatisticsDirectory> is the directory in which all \n"
      << "statistics data are stored. It contains the parameter file "
      << madai::Paths::RUNTIME_PARAMETER_FILE << "\n"
      << "\n"
      << "<OutputFileName> is the name of the comma-separated value-format \n"
      << "file in which the trace will be written. This file will be \n"
      << "written in the directory <StatisticsDirectory>/trace/.\n"
      << "\n"
      << "Format of entries in " << madai::Paths::RUNTIME_PARAMETER_FILE
      << ":\n\n"
      << "MODEL_OUTPUT_DIRECTORY <value> (default: "
      << madai::Defaults::MODEL_OUTPUT_DIRECTORY << ")\n"
      << "EXPERIMENTAL_RESULTS_FILE <value> (default: "
      << madai::Defaults::EXPERIMENTAL_RESULTS_FILE << ")\n"
      << "SAMPLER <value> (default: "
      << madai::Defaults::SAMPLER << ")\n"
      << "SAMPLER_NUMBER_OF_SAMPLES <value> (default: "
      << madai::Defaults::SAMPLER_NUMBER_OF_SAMPLES << ")\n"
      << "MCMC_NUMBER_OF_BURN_IN_SAMPLES <value> (default: "
      << madai::Defaults::MCMC_NUMBER_OF_BURN_IN_SAMPLES << ")\n"
      << "MCMC_USE_MODEL_ERROR <value> (default: "
      << madai::Defaults::MCMC_USE_MODEL_ERROR << ")\n"
      << "MCMC_STEP_SIZE <value> (default: "
      << madai::Defaults::MCMC_STEP_SIZE << ")\n"
      << "EXTERNAL_MODEL_EXECUTABLE <value> (default: \""
      << madai::Defaults::EXTERNAL_MODEL_EXECUTABLE << "\")\n"
      << "EXTERNAL_MODEL_ARGUMENTS <Argument1> <Argument2> ... <LastArgument>\n"
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

  std::string samplerType = settings.GetOption( "SAMPLER", madai::Defaults::SAMPLER );

  int numberOfSamples = settings.GetOptionAsInt(
      "SAMPLER_NUMBER_OF_SAMPLES",
      madai::Defaults::SAMPLER_NUMBER_OF_SAMPLES);

  int numberOfBurnInSamples = settings.GetOptionAsInt(
      "MCMC_NUMBER_OF_BURN_IN_SAMPLES",
      madai::Defaults::MCMC_NUMBER_OF_BURN_IN_SAMPLES);

  bool useModelError = settings.GetOptionAsBool(
      "MCMC_USE_MODEL_ERROR",
      madai::Defaults::MCMC_USE_MODEL_ERROR);

  double stepSize = settings.GetOptionAsDouble(
      "MCMC_STEP_SIZE", madai::Defaults::MCMC_STEP_SIZE);

  std::string executable = settings.GetOption(
      "EXTERNAL_MODEL_EXECUTABLE",
      madai::Defaults::EXTERNAL_MODEL_EXECUTABLE);

  bool verbose = settings.GetOptionAsBool( "VERBOSE", madai::Defaults::VERBOSE );

  madai::ExternalModel externalModel;
  madai::GaussianProcessEmulatedModel gpem;

  madai::Model * model;
  if ( executable == "" ) { // Use emulator

    if ( gpem.LoadConfiguration(
             statisticsDirectory,
             modelOutputDirectory,
             experimentalResultsFile ) != madai::Model::NO_ERROR ) {
      std::cerr << "Error in GaussianProcessEmulatedModel::LoadConfiguration\n";
      return EXIT_FAILURE;
    }

    model = &gpem;

    if ( verbose ) {
      std::cout << "Using emulator to generate trace.\n";
    }
  } else { // Use external model

    // Split arguments into vector of strings
    std::vector< std::string > arguments;
    if ( settings.HasOption( "EXTERNAL_MODEL_ARGUMENTS" ) ) {
      std::string argumentsString =
        settings.GetOption( "EXTERNAL_MODEL_ARGUMENTS" );
      arguments = madai::SplitString( argumentsString, ' ' );
    }

    if ( verbose ) {
      std::cout << "Using external model executable '" << executable << "'.\n";
    }

    externalModel.StartProcess( executable, arguments );
    if (! externalModel.IsReady()) {
      std::cerr << "Something is wrong with the external model\n";
      return EXIT_FAILURE;
    }

    std::ifstream experimentalResults(experimentalResultsFile.c_str());
    if ( madai::Model::NO_ERROR !=
         externalModel.LoadObservations( experimentalResults ) ) {
      std::cerr << "Error loading observations.\n";
      externalModel.StopProcess();

      return EXIT_FAILURE;
    }
    experimentalResults.close();

    model = &externalModel;
  }

  madai::Sampler * sampler;

  madai::PercentileGridSampler pgs;
  madai::MetropolisHastingsSampler mhs;
  if ( samplerType == "PercentileGrid" ) {
    pgs.SetModel( model );

    pgs.SetNumberOfSamples(numberOfSamples);
    numberOfSamples = pgs.GetNumberOfSamples();
    if ( verbose ) {
      std::cout << "Number of grid samples: " << numberOfSamples << "\n";
    }

    // Burn-in samples don't exist for a percentile grid sampling
    numberOfBurnInSamples = 0;

    sampler = &pgs;

    if ( verbose ) {
      std::cout << "Using PercentileGridSampler for sampling\n";
    }
  } else { // Default to Metropolis Hastings
    mhs.SetStepSize( stepSize );

    sampler = &mhs;

    if ( verbose ) {
      std::cout << "Using MetropolisHastingsSampler for sampling\n";
    }
  }

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

  std::ostream * progressStream = verbose ? (& std::cerr) : NULL;
  int returnCode = madai::SamplerCSVWriter::GenerateSamplesAndSaveToFile(
      *sampler,
      *model,
      outFile,
      numberOfSamples,
      numberOfBurnInSamples,
      useModelError,
      progressStream);

  if ( verbose ) {
    if ( returnCode == EXIT_SUCCESS ) {
      std::cout << "Succeeded writing trace file '" << outputFilePath << "'.\n";
    } else {
      std::cerr << "Could not write trace file '" << outputFilePath << "'.\n";
    }
  }

  return returnCode;
}
