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

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include "ApplicationUtilities.h"
#include "Defaults.h"
#include "ExternalModel.h"
#include "MetropolisHastingsSampler.h"
#include "GaussianProcessEmulator.h"
#include "GaussianProcessEmulatorDirectoryFormatIO.h"
#include "GaussianProcessEmulatedModel.h"
#include "Paths.h"
#include "PercentileGridSampler.h"
#include "RuntimeParameterFileReader.h"
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
      << "SAMPLER_INACTIVE_PARAMETERS_FILE <value> (default: "
      << madai::Defaults::SAMPLER_INACTIVE_PARAMETERS_FILE << ")\n"
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

  bool writeLogLikelihoodGradients = settings.GetOptionAsBool(
      "WRITE_LOGLIKELIHOOD_GRADIENTS", 
      madai::Defaults::WRITE_LOGLIKELIHOOD_GRADIENTS );

  bool compressed = settings.GetOptionAsBool( "COMPRESS_TRACE", madai::Defaults::COMPRESS_TRACE );

  madai::ExternalModel externalModel;
  madai::GaussianProcessEmulatedModel gpem;

  madai::Model * model;
  if ( executable == "" ) { // Use emulator
    bool useModelError = settings.GetOptionAsBool(
        "PCA_USE_MODEL_ERROR", madai::Defaults::PCA_USE_MODEL_ERROR );
    madai::GaussianProcessEmulator gpe(useModelError);
    madai::GaussianProcessEmulatorDirectoryFormatIO directoryReader;
    if ( !directoryReader.LoadTrainingData( &gpe,
                                            modelOutputDirectory,
                                            statisticsDirectory,
                                            experimentalResultsFile ) ) {
      std::cerr << "Error loading training data from the directory structure.\n";
      return EXIT_FAILURE;
    }
    if ( !directoryReader.LoadPCA( &gpe, statisticsDirectory ) ) {
      std::cerr << "Error loading the PCA decomposition data. Did you "
                << "run madai_pca_decompose?\n";
      return EXIT_FAILURE;
    }
    if ( !directoryReader.LoadEmulator( &gpe, statisticsDirectory ) ) {
      std::cerr << "Error loading emulator data. Did you run "
                << "madai_train_emulator?\n";
      return EXIT_FAILURE;
    }

    gpem.SetGaussianProcessEmulator( gpe );

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

    model = &externalModel;
  }

  std::ifstream experimentalResults(experimentalResultsFile.c_str());
  if ( madai::Model::NO_ERROR !=
       madai::LoadObservations( model, experimentalResults ) ) {
    std::cerr << "Error loading observations.\n";
    externalModel.StopProcess();
    return EXIT_FAILURE;
  }
  experimentalResults.close();

  madai::Sampler * sampler;

  madai::PercentileGridSampler pgs;
  madai::MetropolisHastingsSampler mhs;
  if ( samplerType == "PercentileGrid" ) {
    pgs.SetModel( model );

    // Burn-in samples don't exist for a percentile grid sampling
    numberOfBurnInSamples = 0;

    sampler = &pgs;

    if ( verbose ) {
      std::cout << "Using PercentileGridSampler for sampling\n";
    }
  } else { // Default to Metropolis Hastings
    mhs.SetModel( model );
    mhs.SetStepSize( stepSize );

    sampler = &mhs;

    if ( verbose ) {
      std::cout << "Using MetropolisHastingsSampler for sampling\n";
    }
  }

  // Potentially set some parameters to inactive
  std::string samplerInactiveParametersFile =
    madai::GetInactiveParametersFile( statisticsDirectory, settings );
  if ( samplerInactiveParametersFile != "" ) {
    if ( ! madai::SetInactiveParameters( samplerInactiveParametersFile,
                                         *sampler, verbose ) ) {
      std::cerr << "Error when setting inactive parameters from file '"
                << samplerInactiveParametersFile << "'.\n";
      return EXIT_FAILURE;
    }
  }

  if ( samplerType == "PercentileGrid" ) {
    assert( pgs.GetModel() != NULL );
    pgs.SetNumberOfSamples(numberOfSamples);
    numberOfSamples = pgs.GetNumberOfSamples();
    if ( verbose ) {
      std::cout << "Number of grid samples: " << numberOfSamples << "\n";
    }
  }

  std::string outputFilePath( argv[2] );
  std::ofstream outFile(outputFilePath.c_str(), std::ios_base::out | std::ios_base::binary);
  if ( !outFile.good() ) {
    std::cerr << "Could not open trace file '" << outputFilePath << "' for writing.\n";
    return EXIT_FAILURE;
  }
  boost::iostreams::filtering_streambuf<boost::iostreams::output> outbuf;
  if( compressed ) {
      outbuf.push(boost::iostreams::gzip_compressor());
  }
  outbuf.push(outFile);
  std::ostream outFileStream(&outbuf);

  std::ostream * progressStream = verbose ? (& std::cerr) : NULL;
  int returnCode = madai::SamplerCSVWriter::GenerateSamplesAndSaveToFile(
      *sampler,
      *model,
      outFileStream,
      numberOfSamples,
      numberOfBurnInSamples,
      useModelError,
      writeLogLikelihoodGradients,
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
