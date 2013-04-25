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
#include "Trace.h"

#include "madaisys/SystemTools.hxx"

using madai::Paths;

static const int    DEFAULT_MCMC_NUMBER_OF_SAMPLES         = 100;
static const int    DEFAULT_MCMC_NUMBER_OF_BURN_IN_SAMPLES = 0;
static const bool   DEFAULT_MCMC_USE_EMULATOR_COVARIANCE   = false;
static const double DEFAULT_MCMC_STEP_SIZE                 = 0.1;


int main(int argc, char ** argv) {

  if (argc < 3) {
    std::cerr << "Useage:\n"
              << "    generateMCMCtrace <StatisticsDirectory> <OutputFileName>\n"
              << "\n"
              << "This program produces a Markov Chain Monte Carlo trace from \n"
              << "a trained emulator. The programs PCADecompose and basicTrain \n"
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
              << Paths::DEFAULT_MODEL_OUTPUT_DIRECTORY << ")\n"
              << "EXPERIMENTAL_RESULTS_DIRECTORY <value> (default: "
              << Paths::DEFAULT_EXPERIMENTAL_RESULTS_DIRECTORY << ")\n"
              << "MCMC_NUMBER_OF_SAMPLES <value> (default: "
              << DEFAULT_MCMC_NUMBER_OF_SAMPLES << ")\n"
              << "MCMC_NUMBER_OF_BURN_IN_SAMPLES <value> (default: "
              << DEFAULT_MCMC_NUMBER_OF_BURN_IN_SAMPLES << ")\n"
              << "MCMC_USE_EMULATOR_COVARIANCE <value> (default: "
              << DEFAULT_MCMC_USE_EMULATOR_COVARIANCE << ")\n"
              << "MCMC_STEP_SIZE <value> (default: "
              << DEFAULT_MCMC_STEP_SIZE << ")\n";

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

  int numberOfSamples = DEFAULT_MCMC_NUMBER_OF_SAMPLES;
  if ( settings.HasOption( "MCMC_NUMBER_OF_SAMPLES" ) ) {
    numberOfSamples = atoi( settings.GetOption( "MCMC_NUMBER_OF_SAMPLES" ).c_str() );
  }
  int numberOfBurnInSamples = DEFAULT_MCMC_NUMBER_OF_BURN_IN_SAMPLES;
  if ( settings.HasOption( "MCMC_NUMBER_OF_BURN_IN_SAMPLES" ) ) {
    numberOfBurnInSamples =
      atoi( settings.GetOption( "MCMC_NUMBER_OF_BURN_IN_SAMPLES" ).c_str() );
  }
  bool useEmulatorCovariance = DEFAULT_MCMC_USE_EMULATOR_COVARIANCE;
  if ( settings.HasOption( "MCMC_USE_EMULATOR_COVARIANCE" ) ) {
    useEmulatorCovariance = ( settings.GetOption( "MCMC_USE_EMULATOR_COVARIANCE" ) == "true" );
  }
  double stepSize = DEFAULT_MCMC_STEP_SIZE;
  if ( settings.HasOption( "MCMC_STEP_SIZE" ) ) {
    stepSize = atof( settings.GetOption( "MCMC_STEP_SIZE" ).c_str() );
  }

  std::string observationsFile = experimentalResultsDirectory +
    madai::Paths::SEPARATOR + madai::Paths::RESULTS_FILE;
  
  madai::GaussianProcessEmulatedModel gpem;
  if ( gpem.LoadConfiguration( statisticsDirectory,
                               modelOutputDirectory,
                               experimentalResultsDirectory ) != madai::Model::NO_ERROR ) {
    std::cerr << "Error in GaussianProcessEmulatedModel::LoadConfiguration\n";
    return EXIT_FAILURE;
  }

  gpem.SetUseModelCovarianceToCalulateLogLikelihood( useEmulatorCovariance );

  std::ifstream observations( observationsFile.c_str() );
  if ( madai::Model::NO_ERROR != gpem.LoadObservations( observations ) ) {
    std::cerr << "Error loading observations.\n";
    return EXIT_FAILURE;
  }
  observations.close();

  madai::MetropolisHastingsSampler mcmc;
  mcmc.SetModel( &gpem );
  mcmc.SetStepSize( stepSize );

  int step = numberOfBurnInSamples / 100, percent = 0;
  if ( step < 1 ) {
    step = 1; // avoid div-by-zero error
  }

  for ( int count = 0; count < numberOfBurnInSamples; count++ ) {
    if ( count % step == 0 ) {
      std::cout << '\r' << "Burn in percent done: " << percent++ << "%";
    }

    // Discard samples in the burn-in phase
    mcmc.NextSample();
  }
  step = numberOfSamples / 100, percent = 0;
  if ( step < 1 ) {
    step = 1; // avoid div-by-zero error
  }

  madai::Trace trace;
  for (int count = 0; count < numberOfSamples; count ++) {
    if (count % step == 0)
      std::cout <<  '\r' << "MCMC percent done: " << percent++ << "%";
    trace.Add( mcmc.NextSample() );
  }
  std::cout << "\r" ;

  std::string traceDirectory = statisticsDirectory + madai::Paths::TRACE_DIRECTORY;
  madaisys::SystemTools::MakeDirectory( traceDirectory.c_str() );
  std::string outputFileName( argv[2] );
  std::string outputFilePath = traceDirectory + Paths::SEPARATOR + outputFileName;
  std::ofstream out( outputFilePath.c_str() );
  trace.WriteCSVOutput( out,
                        gpem.GetParameters(),
                        gpem.GetScalarOutputNames() );

  return EXIT_SUCCESS;
}
