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

#include "ApplicationUtilities.h"
#include "MetropolisHastingsSampler.h"
#include "RuntimeParameterFileReader.h"
#include "ExternalModel.h"
#include "Trace.h"
#include "Paths.h"

#include "madaisys/SystemTools.hxx"

/**
generateMCMCtraceExternal
  Generate a trace of length N for an External Process
 */
static const int DEFAULT_NUMBER_ITERATIONS = 100;
static const int DEFAULT_BURN_IN = 0;
static const double DEFAULT_STEP_SIZE = 0.1;
const char useage [] =
  "Usage:\n"
  "    generateMCMCtraceExternal StatisticsDirectory OutputFileName\n"
  "\n"
  "StatisticsDirectory is the directory in which all statistical data will\n"
  "be stored. Contains the parameter file stat_params.dat\n"
  "\n"
  "OutputFileName is the nameof the file the trace will be storedin. This file\n"
  "will be located in the StatisticsDirectory/trace/ directory.\n"
  "\n"
  "Format of stat_params.dat\n"
  "EXPERIMENTAL_RESULTS_DIRECTORY <value>\n"
  "MCMC_NUMBER_ITERATIONS <value\n"
  "MCMC_NUMBER_BURN_IN <value>\n"
  "MCMC_USE_MODEL_ERROR <value>\n"
  "MCMC_STEP_SIZE <value>\n"
  "EXTERNAL_MODEL_ARGUMENTS\n"
  "<Argument1>\n"
  "<Argument2>\n"
  "...\n"
  "<LastArgument>\n"
  "ARGUMENTS_DONE\n"
  "\n";

struct ExternalModelMCMCRuntimeParameters
{
  int numberIter;
  int numberBurnIn;
  bool UseModelError;
  double StepSize;
  std::string ModelOutputDirectory;
  std::string ExperimentalResultsDirectory;
  std::string executable;
  std::vector< std::string > arguments;
};

bool parseEMMCMCRuntimeParameters(
    int argc, char** argv,
    struct ExternalModelMCMCRuntimeParameters & Opts )
{
  // Initialize as defaults
  Opts.numberIter = DEFAULT_NUMBER_ITERATIONS;
  Opts.numberBurnIn = DEFAULT_BURN_IN;
  Opts.UseModelError = false;
  Opts.StepSize = DEFAULT_STEP_SIZE;

  for ( unsigned int i = 0; i < argc; i++ ) {
    std::string argString( argv[i] );

    if ( argString == "MCMC_NUMBER_ITERATIONS" ) {
      Opts.numberIter = atoi(argv[i+1]);
      i++;
    } else if ( argString == "EXPERIMENTAL_RESULTS_DIRECTORY" ) {
      Opts.ExperimentalResultsDirectory = std::string( argv[i+1] );
      i++;
    } else if ( argString == "MCMC_NUMBER_BURN_IN" ) {
      Opts.numberBurnIn = atoi(argv[i+1]);
      i++;
    } else if ( argString == "MCMC_USE_MODEL_ERROR" ) {
      std::string tstring(argv[i+1]);
      if ( tstring == "false" || tstring == "0" ) {
        Opts.UseModelError = false;
      } else if ( tstring == "true" || tstring == "1" ) {
        Opts.UseModelError = true;
      } else {
        std::cerr << "MCMC_USE_MODEL_ERROR: " << tstring << " is invalid\n"
        << "Setting to false\n";
        Opts.UseModelError = false;
      }
      i++;
    } else if ( argString == "MCMC_STEP_SIZE" ) {
      Opts.StepSize = atof(argv[i+1]);
      i++;
    } else if ( argString == "EXTERNAL_MODEL_ARGUMENTS" ) {
      bool Done = false;
      while ( !Done ) {
        if ( i == (argc - 1) ) {
          std::cerr << "Reached end of runtime parameter list without reaching\n"
                    << "the end of the arguments for the external model\n";
          return false;
        }
        std::string tstring( argv[i+1] );
        if ( tstring == "ARGUMENTS_DONE" ) break;
        Opts.arguments.push_back( tstring );
        i++;
      }
      i++;
    }
  }
  return true;
}

int main(int argc, char ** argv) {

  if ( argc < 3 ) {
     std::cerr << useage << '\n';
     return EXIT_FAILURE;
  }
  std::string StatisticsDirectory(argv[1]);
  madai::EnsurePathSeparatorAtEnd( StatisticsDirectory );
  std::string OutputFileName(argv[2]);
  madai::RuntimeParameterFileReader RPFR;
  RPFR.ParseFile( StatisticsDirectory + madai::Paths::RUNTIME_PARAMETER_FILE );
  char** Args = RPFR.GetArguments();
  int NArgs = RPFR.GetNumberOfArguments();
  struct ExternalModelMCMCRuntimeParameters Opts;
  if ( !parseEMMCMCRuntimeParameters( NArgs, Args, Opts ) ) {
    std::cerr << "Error parsing configuration file for external model mcmc.\n";
    return EXIT_FAILURE;
  }

  madai::ExternalModel em;
  em.StartProcess( Opts.executable, Opts.arguments );
  if (! em.IsReady()) {
    std::cerr << "Something is wrong with the external model\n";
    return EXIT_FAILURE;
  }

  em.SetUseModelCovarianceToCalulateLogLikelihood(Opts.UseModelError);
  
  std::string observationsFile = StatisticsDirectory + Opts.ExperimentalResultsDirectory +
    madai::Paths::SEPARATOR + madai::Paths::RESULTS_FILE;
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

  madai::Trace trace;
  for (std::vector< madai::Sample >::const_iterator it = samples.begin() ;
       it != samples.end(); ++it) {
    trace.Add( *it );
  }

  std::string traceDirectory = StatisticsDirectory + madai::Paths::TRACE_DIRECTORY;
  madaisys::SystemTools::MakeDirectory( traceDirectory.c_str() );
  std::string OutputFile = traceDirectory + madai::Paths::SEPARATOR + OutputFileName;
  std::ofstream Out( OutputFile.c_str() );
  trace.WriteCSVOutput(
      Out,
      em.GetParameters(),
      em.GetScalarOutputNames() );
  return EXIT_SUCCESS;
}
