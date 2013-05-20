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
#include "MetropolisHastingsSampler.h"
#include "GaussianProcessEmulatedModel.h"
#include "RuntimeParameterFileReader.h"
#include "Paths.h"
#include "Defaults.h"
#include "Trace.h"
#include "SamplerCSVWriter.h"

#include "madaisys/SystemTools.hxx"

using madai::Paths;


int main(int argc, char ** argv) {

  if (argc < 3) {
    std::cerr
      << "Usage:\n"
      << "    " << argv[0] << " <StatisticsDirectory> <OutputFileName>\n"
      << "\n"
      << "This program produces a Markov Chain Monte Carlo trace from \n"
      << "a trained emulator. The programs madai_pca_decompose and madai_basic_train \n"
      << "must have been run on <StatisticsDirectory> prior to running \n"
      << "this program.\n"
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
      << "MCMC_NUMBER_OF_SAMPLES <value> (default: "
      << madai::Defaults::MCMC_NUMBER_OF_SAMPLES << ")\n"
      << "MCMC_NUMBER_OF_BURN_IN_SAMPLES <value> (default: "
      << madai::Defaults::MCMC_NUMBER_OF_BURN_IN_SAMPLES << ")\n"
      << "MCMC_USE_MODEL_ERROR <value> (default: "
      << madai::Defaults::MCMC_USE_MODEL_ERROR << ")\n"
      << "MCMC_STEP_SIZE <value> (default: "
      << madai::Defaults::MCMC_STEP_SIZE << ")\n"
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

  int numberOfSamples = madai::Defaults::MCMC_NUMBER_OF_SAMPLES;
  if ( settings.HasOption( "MCMC_NUMBER_OF_SAMPLES" ) ) {
    numberOfSamples = atoi( settings.GetOption( "MCMC_NUMBER_OF_SAMPLES" ).c_str() );
  }
  int numberOfBurnInSamples = madai::Defaults::MCMC_NUMBER_OF_BURN_IN_SAMPLES;
  if ( settings.HasOption( "MCMC_NUMBER_OF_BURN_IN_SAMPLES" ) ) {
    numberOfBurnInSamples =
      atoi( settings.GetOption( "MCMC_NUMBER_OF_BURN_IN_SAMPLES" ).c_str() );
  }
  bool useModelError = madai::Defaults::MCMC_USE_MODEL_ERROR;
  if ( settings.HasOption( "MCMC_USE_MODEL_ERROR" ) ) {
    useModelError = ( settings.GetOption( "MCMC_USE_MODEL_ERROR" ) == "true" );
  }
  double stepSize = madai::Defaults::MCMC_STEP_SIZE;
  if ( settings.HasOption( "MCMC_STEP_SIZE" ) ) {
    stepSize = atof( settings.GetOption( "MCMC_STEP_SIZE" ).c_str() );
  }

  madai::GaussianProcessEmulatedModel gpem;
  if ( gpem.LoadConfiguration(
           statisticsDirectory,
           modelOutputDirectory,
           experimentalResultsFile ) != madai::Model::NO_ERROR ) {
    std::cerr << "Error in GaussianProcessEmulatedModel::LoadConfiguration\n";
    return EXIT_FAILURE;
  }

  madai::MetropolisHastingsSampler mcmc;
  mcmc.SetStepSize( stepSize );

  std::string traceDirectory =
    statisticsDirectory + madai::Paths::TRACE_DIRECTORY;
  madaisys::SystemTools::MakeDirectory( traceDirectory.c_str() );
  std::string outputFileName( argv[2] );
  std::string outputFilePath =
    traceDirectory + Paths::SEPARATOR + outputFileName;

  std::ofstream outFile(outputFilePath.c_str());
  if ( !outFile.good() ) {
    std::cerr << "Could not open trace file '" << outputFilePath << "' for writing.\n";
    return EXIT_FAILURE;
  }

  int returnCode = madai::SamplerCSVWriter::GenerateSamplesAndSaveToFile(
      mcmc,
      gpem,
      outFile,
      numberOfSamples,
      numberOfBurnInSamples,
      useModelError,
      &(std::cerr));

  if ( settings.GetOptionAsBool( "VERBOSE", madai::Defaults::VERBOSE ) ) {
    if ( returnCode == EXIT_SUCCESS ) {
      std::cout << "Succeeded writing trace file '" << outputFilePath << "'.\n";
    } else {
      std::cerr << "Could not write trace file '" << outputFilePath << "'.\n";
    }
  }

  return returnCode;
}
