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

const int DEFAULT_NUMBER_ITERATIONS = 100;
const char useage [] =
  "Useage:\n"
  "    generatePercentileGridTraceExternal StatisticsDirectory OutputFileName\n"
  "\n"
  "StatisticsDirectory is the directory in which all statistical data will\n"
  "be stored. Contains the parameter file stat_params.dat:\n"
  "\n"
  "Format of and parameters which can be set in stat_params.dat:\n"
  "EXPERIMENTAL_RESULTS_DIRECTORY <value>\n"
  "PERCENTILE_GRID_NUMBER_ITERATIONS <value>\n"
  "EXTERNAL_MODEL_ARGUMENTS\n"
  "<Argument1>\n"
  "<Argument2>\n"
  "...\n"
  "<LastAgument>\n"
  "ARGUMENTS_DONE\n"
  "\n";

struct EMPercentileGridRuntimeParameters
{
  int numberIter;
  std::string ExperimentalResultsDirectory;
  std::string executable;
  std::vector< std::string > arguments;
};

bool parseEMPGRuntimeParameters(
    int argc, char** argv,
    struct EMPercentileGridRuntimeParameters & Opts )
{
  // Initialize as defaults
  Opts.numberIter = DEFAULT_NUMBER_ITERATIONS;
  Opts.ExperimentalResultsDirectory = madai::Paths::DEFAULT_EXPERIMENTAL_RESULTS_DIRECTORY;
  
  for ( unsigned int i = 0; i < argc; i++ ) {
    std::string argString( argv[i] );
    
    if ( argString == "EXPERIMENTAL_RESULTS_DIRECTORY" ) {
      Opts.ExperimentalResultsDirectory = std::string( argv[i+1] );
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
    } else if ( argString == "EXTERNAL_MODEL_EXECUTABLE" ) {
      Opts.executable = std::string( argv[i+1] );
      i++;
    }
  }
  return true;
}

template<class S, class T>
int findIndex(const S & v, const T & s)
{
  typename S::const_iterator it = std::find(v.begin(), v.end(), s);
  if (it == v.end())
    return -1;
  return std::distance(v.begin(), it);
}

using madai::Model;
/**
   Load a file with experimental observations in it.  The model will
   be comared against this. */
Model::ErrorType
LoadObservations(Model * model, std::istream & i)
{
  // std::ifstream i("DIRECTORY/experimental_results/results.dat");
  const std::vector< std::string > & scalarOutputNames = model->GetScalarOutputNames();
  unsigned int numberOfScalarOutputs = model->GetNumberOfScalarOutputs();
  assert(scalarOutputNames.size() == numberOfScalarOutputs);
  assert (numberOfScalarOutputs > 0);
  std::vector< double > observedScalarValues(numberOfScalarOutputs, 0.0);
  std::vector< double > observedScalarCovariance(
      numberOfScalarOutputs * numberOfScalarOutputs, 0.0);
  for (unsigned int j = 0; j < numberOfScalarOutputs; ++j)
    observedScalarCovariance[j * (1 + numberOfScalarOutputs)] = 1.0;
  while (true) { // will loop forever if input stream lasts forever.
    std::string name;
    double value, uncertainty;
    if(! (i >> name >> value >> uncertainty))
      break;
    int index = findIndex(scalarOutputNames, name);
    if (index != -1) {
      observedScalarValues[index] = value;
      // observedScalarCovariance is a square matrix;
      observedScalarCovariance[index * (1 + numberOfScalarOutputs)]
        = std::pow(uncertainty, 2);
      // uncertainty^2 is variance.
    }
  }
  // assume extra values are all zero.
  Model::ErrorType e;
  e = model->SetObservedScalarValues(observedScalarValues);
  if (e != madai::Model::NO_ERROR) {
    std::cerr << "Error in Model::SetObservedScalarValues\n";
    return e;
  }
  e = model->SetObservedScalarCovariance(observedScalarCovariance);
  if (e != madai::Model::NO_ERROR) {
    std::cerr << "Error in Model::SetObservedScalarCovariance\n";
    return e;
  }
  return madai::Model::NO_ERROR;
}


/**
   \fixme document
 */
int main(int argc, char ** argv) {

  if (argc < 3) {
    std::cerr << useage;
    return EXIT_FAILURE;
  }
  std::string StatisticsDirectory( argv[1] );
  madai::EnsurePathSeparatorAtEnd( StatisticsDirectory );
  std::string OutputFileName( argv[2] );
  madai::RuntimeParameterFileReader RPFR;
  RPFR.ParseFile( StatisticsDirectory + madai::Paths::RUNTIME_PARAMETER_FILE );
  char** Args = RPFR.GetArguments();
  int NArgs = RPFR.GetNumberOfArguments();
  struct EMPercentileGridRuntimeParameters Opts;
  if ( !parseEMPGRuntimeParameters( NArgs, Args, Opts ) ) {
    std::cerr << "error parsing configuration file for external model mcmc.\n";
    return EXIT_FAILURE;
  }

  madai::ExternalModel em;
  em.StartProcess( Opts.executable, Opts.arguments );
  if (! em.IsReady()) {
    std::cerr << "Something is wrong with the external model\n";
    return EXIT_FAILURE;
  }

  em.SetUseModelCovarianceToCalulateLogLikelihood(false);

  std::string observationsFile = StatisticsDirectory + 
    Opts.ExperimentalResultsDirectory + madai::Paths::SEPARATOR + 
    madai::Paths::RESULTS_FILE;
  std::ifstream observations(observationsFile.c_str());
  if (madai::Model::NO_ERROR != LoadObservations(&em, observations)) {
    std::cerr << "error loading observations.\n";
    em.StopProcess();
    return EXIT_FAILURE;
  }
  observations.close();

  madai::PercentileGridSampler sampler;
  sampler.SetModel( &em );
  sampler.SetNumberSamples(Opts.numberIter);
  int numberIter = sampler.GetNumberSamples();

  std::vector< madai::Parameter > const & parameters
    = em.GetParameters();

  madai::Trace trace;

  int step = numberIter / 100, percent = 0;
  for (int count = 0; count < numberIter; count ++) {
    if (count % step == 0)
      std::cerr <<  '\r' << percent++ << "%";
    trace.Add(sampler.NextSample());
  }
  std::cerr << "\r" ;

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
