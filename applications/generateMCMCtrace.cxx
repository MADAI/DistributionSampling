/*=========================================================================
 *
 *  Copyright 2011-2013 The University of North Carolina at Chapel Hill
 *  All rights reserved.
 *
 *  Licensed under the MADAI Software License. You may obtain a copy of
 *  this license at
 *
 *         https://madai-public.cs.unc.edu/software/license/
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

#include "MetropolisHastingsSampler.h"
#include "GaussianProcessEmulatedModel.h"
#include "Trace.h"

/**
generateMCMCtrace
  Generate a trace of length N for a Gaussian Process Model Emulator
 */
static const int DEFAULT_NUMBER_ITERATIONS = 100;
static const int DEFAULT_BURN_IN = 0;
static const double DEFAULT_STEP_SIZE = 0.1;
const char useage [] =
  "Usage:\n"
  "    generateMCMCtrace TopDirectory OutputFileName\n"
  "\n"
  "TopDirectory is the directory containing model_output/ experimental_results/\n"
  "and statistical_analysis/.\n"
  "\n"
  "OutputFileName is the name of the file the trace will be stored in. This file\n"
  "will be located in the statistical_analysis/MCMCTrace/ directory.\n";
 
struct GaussianProcessMCMCRuntimeParameters
{
  int numberIter;
  int numberBurnIn;
  bool UseEmulatedCovariance;
  double StepSize;
};

bool parseGaussianProcessMCMCConfig( 
    std::istream & input, 
    struct GaussianProcessMCMCRuntimeParameters & Opts ) 
{
  // Initialize as defaults
  Opts.numberIter = DEFAULT_NUMBER_ITERATIONS;
  Opts.numberBurnIn = DEFAULT_BURN_IN;
  Opts.UseEmulatedCovariance = false;
  Opts.StepSize = DEFAULT_STEP_SIZE;
  
  std::string name, tstring;
  char opt;
  std::vector< std::string > ParamNames;
  while ( input >> name ) {
    if ( name == "ITERATIONS" ) {
      opt = 'N';
    } else if ( name == "BURN_IN" ) {
      opt = 'B';
    } else if ( name == "USE_EMULATED_ERROR" ) {
      opt = 'U';
    } else if ( name == "STEP_SIZE" ) {
      opt = 'S';
    }
    switch( opt ) {
    case 'N':
      int Niter;
      input >> Niter;
      Opts.numberIter = Niter;
      if ( Opts.numberIter < 0 ) {
        std::cerr << "Error: ITERATIONS given incorrect argument: \""
          << Niter << "\"\n";
        return false;
      }
      break;
    case 'B':
      int NBurn;
      input >> NBurn;
      Opts.numberBurnIn = NBurn;
      if ( Opts.numberBurnIn < 0 ) {
        std::cerr << "Error: BURN_IN given incorrect argument: \""
          << NBurn << "\"\n";
        return false;
      }
      break;
    case 'U':
      input >> tstring;
      if ( tstring == "false" ) {
        Opts.UseEmulatedCovariance = false;
      } else if ( tstring == "true" ) {
        Opts.UseEmulatedCovariance = true;
      } else {
        std::cerr << "Error: USE_EMULATED_ERROR given incorrect argument: \""
          << tstring << "\"\n";
        return false;
      }
      break;
    case 'S':
      double SS;
      input >> SS;
      Opts.StepSize = SS;
      if ( Opts.StepSize < 0 ) {
        std::cerr << "Error: STEP_SIZE given incorrect argument: \""
          << SS << "\"\n";
        return false;
      }
      break;
    }
  }
  return true;
}
 
 
int main(int argc, char ** argv) {

  if (argc < 3) {
    std::cerr << useage << "\n";
    return EXIT_FAILURE;
  }
  std::string TopDirectory(argv[1]);
  std::string OutputFileName(argv[2]);
  std::string observationsFile = TopDirectory+"/experimental_results/results.dat";
  std::string RuntimeParametersFileName = TopDirectory+"/statistical_analysis/MCMC.param";
  
  std::ifstream RPF ( RuntimeParametersFileName.c_str() );
  struct GaussianProcessMCMCRuntimeParameters Opts;
  if ( !parseGaussianProcessMCMCConfig( RPF, Opts ) ) {
    std::cerr << "Error: Parsing configuration file for mcmc.\n";
    return EXIT_FAILURE;
  }
  
  madai::GaussianProcessEmulatedModel gpem;
  if (gpem.LoadConfiguration( TopDirectory ) != madai::Model::NO_ERROR) {
    std::cerr << "Error in GaussianProcessEmulatedModel::LoadConfiguration\n";
    return EXIT_FAILURE;
  }

  gpem.SetUseModelCovarianceToCalulateLogLikelihood(Opts.UseEmulatedCovariance);

  std::ifstream observations(observationsFile.c_str());
  if (madai::Model::NO_ERROR != gpem.LoadObservations(observations)) {
    std::cerr << "error loading observations.\n";
    return EXIT_FAILURE;
  }
  observations.close();

  madai::MetropolisHastingsSampler mcmc;
  mcmc.SetModel( &gpem );
  mcmc.SetStepSize(Opts.StepSize);

  std::vector< madai::Parameter > const & parameters
    = gpem.GetParameters();

  int t = gpem.GetNumberOfScalarOutputs();

  int step = Opts.numberBurnIn / 100, percent = 0;
  for ( int count = 0; count < Opts.numberBurnIn; count++ ) {
    if ( count % step == 0 )
      std::cerr << '\r' << "Burn in done: " << percent++ << "%";
    mcmc.NextSample();
  }
  step = Opts.numberIter / 100, percent = 0;
  std::vector< madai::Sample> samples;
  for (int count = 0; count < Opts.numberIter; count ++) {
    if (count % step == 0)
      std::cerr <<  '\r' << "MCMC percent done: " << percent++ << "%";
    samples.push_back(mcmc.NextSample());
  }
  std::cerr << "\r" ;

  std::sort(samples.begin(),samples.end());

  madai::Trace trace;
  for (std::vector< madai::Sample >::const_iterator it = samples.begin() ;
       it != samples.end(); ++it) {
    trace.Add( *it );
  }
  std::string command = "mkdir -p "+TopDirectory+"/statistical_analysis/MCMCTrace";
  std::system(command.c_str());
  std::string OutputFile = TopDirectory+"/statistical_analysis/MCMCTrace/"+OutputFileName;
  std::ofstream Out( OutputFile.c_str() );
  trace.WriteCSVOutput(
      Out,
      gpem.GetParameters(),
      gpem.GetScalarOutputNames() );
  return EXIT_SUCCESS;
}
