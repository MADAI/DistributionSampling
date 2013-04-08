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
#include "ExternalModel.h"
#include "Trace.h"


/**
generateMCMCtraceExternal
  Generate a trace of length N for an External Process
 */
static const int DEFAULT_NUMBER_ITERATIONS = 100;
static const int DEFAULT_BURN_IN = 0;
static const double DEFAULT_STEP_SIZE = 0.1;
const char useage [] =
  "Usage:\n"
  "    generateMCMCtraceExternal TopDirectory OutputFileName\n"
  "\n"
  "TopDirectory is the directory containing model_output/ experimental_results/\n"
  "and statistical_analysis/.\n"
  "\n"
  "OutputFileName is the nameof the file the trace will be storedin. This file\n"
  "will be located in the statistical_analsis/MCMCTrace/ directory.\n";

struct ExternalModelMCMCRuntimeParameters
{
  int numberIter;
  int numberBurnIn;
  bool UseModelError;
  bool UseModelSnapshotFile;
  double StepSize;
  std::string executable;
  std::vector< std::string > arguments;
};

bool parseExternalModelMCMCConfig(
    std::istream & input,
    struct ExternalModelMCMCRuntimeParameters & Opts )
{
  // Initialize as defaults
  Opts.numberIter = DEFAULT_NUMBER_ITERATIONS;
  Opts.numberBurnIn = DEFAULT_BURN_IN;
  Opts.UseModelError = false;
  Opts.UseModelSnapshotFile = false;
  Opts.StepSize = DEFAULT_STEP_SIZE;
  
  std::string name, tstring;
  char opt;
  while ( input.good() ) {
    while ( input.peek() == '#' ) {
      std::string line;
      std::getline( input, line );
      std::cerr << line << std::endl;
    }
    input >> name;
    if ( name == "ITERATIONS" ) {
      opt = 'N';
    } else if ( name == "BURN_IN" ) {
      opt = 'B';
    } else if ( name == "USE_MODEL_ERROR" ) {
      opt = 'E';
    } else if ( name == "STEP_SIZE" ) {
      opt = 'S';
    } else if ( name == "USE_MODEL_SNAPSHOT_FILE" ) {
      opt = 'F';
    } else if ( name == "EXECUTABLE" ) {
      opt = 'X';
    } else if ( name == "ARGUMENTS" ) {
      opt = 'A';
    } else {
      std::cerr << "Unknown parameter " << name << '\n';
      opt = NULL;
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
    case 'E':
      input >> tstring;
      if ( tstring == "false" ) {
        Opts.UseModelError = false;
      } else if ( tstring == "true" ) {
          Opts.UseModelError = true;
      } else {
        std::cerr << "Error: USE_MODEL_ERROR given incorrect argument: \""
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
    case 'F':
      input >> tstring;
      if ( tstring == "false" ) {
        Opts.UseModelSnapshotFile = false;
      } else if ( tstring == "true" ) {
        Opts.UseModelSnapshotFile = true;
      } else {
        std::cerr << "Error: USE_MODEL_SNAPSHOT_FILE given incorrect argument:\""
          << tstring << "\"\n";
        return false;
      }
      break;
    case 'X':
      input >> tstring;
      Opts.executable = tstring;
      break;
    case 'A':
      input >> tstring;
      while ( tstring != "ARGS_DONE" ) {
        Opts.arguments.push_back(tstring);
      }
      break;
    }
  }
  return true;
}

int main(int argc, char ** argv) {

  if ( argc < 3 ) {
     std::cerr << useage << '\n';
     return EXIT_FAILURE;
  }
  std::string TopDirectory(argv[1]);
  std::string OutputFileName(argv[2]);
  std::string observationsFile = TopDirectory+"/experimental_results/results.dat";
  std::string RuntimeParametersFileName = TopDirectory+"/statistical_analysis/MCMC.dat";
  
  std::ifstream RPF ( RuntimeParametersFileName.c_str() );
  struct ExternalModelMCMCRuntimeParameters Opts;
  if ( !parseExternalModelMCMCConfig( RPF, Opts ) ) {
    std::cerr << "Error: Parsing configuration file for external model mcmc.\n";
    return EXIT_FAILURE;
  }
  
  madai::ExternalModel em;
  em.StartProcess( Opts.executable, Opts.arguments );
  if (! em.IsReady()) {
    std::cerr << "Something is wrong with the external model\n";
    return EXIT_FAILURE;
  }

  em.SetUseModelCovarianceToCalulateLogLikelihood(Opts.UseModelError);

  std::ifstream observations(observationsFile.c_str());
  if (madai::Model::NO_ERROR != em.LoadObservations(observations)) {
    std::cerr << "error loading observations.\n";
    em.StopProcess();
    return EXIT_FAILURE;
  }
  observations.close();

  madai::MetropolisHastingsSampler mcmc;
  mcmc.SetModel( &em );
  mcmc.SetStepSize(Opts.StepSize);

  std::vector< madai::Parameter > const & parameters
    = em.GetParameters();

  int t = em.GetNumberOfScalarOutputs();

  int step = Opts.numberBurnIn / 100, percent = 0;
  for (int count = 0; count < Opts.numberBurnIn; count++ ) {
    if ( count % step == 0 )
      std::cerr << '\r' << percent++ << "%";
  }
  step = Opts.numberIter / 100, percent = 0;
  std::vector< madai::Sample> samples;
  for (int count = 0; count < Opts.numberIter; count ++) {
    if (count % step == 0)
      std::cerr <<  '\r' << percent++ << "%";
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
      em.GetParameters(),
      em.GetScalarOutputNames() );
  return EXIT_SUCCESS;
}
