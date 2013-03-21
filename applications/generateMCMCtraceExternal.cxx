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

#include "MetropolisHastingsSampler.h"
#include "ExternalModel.h"
#include "Trace.h"


/**
   \fixme document
 */
int main(int argc, char ** argv) {

  if (argc < 4) {
    std::cerr << "Useage:\n  "
      "generateMCMCTraceExternal observationsFile N ExternalARGS....\n\n";
    return EXIT_FAILURE; //\fixme useage
  }
  const char * observationsFile = argv[1];
  int numberIter = atoi(argv[2]);
  std::string executable( argv[3] );
  std::vector< std::string > arguments;
  for (int i = 4 ; i < argc; ++i)
    arguments.push_back(argv[i]);

  madai::ExternalModel em;
  em.StartProcess( executable, arguments );
  if (! em.IsReady()) {
    std::cerr << "Something is wrong with the external model\n";
    return EXIT_FAILURE;
  }

  em.SetUseModelCovarianceToCalulateLogLikelihood(false);

  std::ifstream observations(observationsFile);
  if (madai::Model::NO_ERROR != em.LoadObservations(observations)) {
    std::cerr << "error loading observations.\n";
    em.StopProcess();
    return EXIT_FAILURE;
  }
  observations.close();

  madai::MetropolisHastingsSampler mcmc;
  mcmc.SetModel( &em );
  mcmc.SetStepSize(0.1);

  std::vector< madai::Parameter > const & parameters
    = em.GetParameters();

  int t = em.GetNumberOfScalarOutputs();

  std::vector< madai::Sample> samples;
  for (int count = 0; count < numberIter; count ++)
    samples.push_back(mcmc.NextSample());

  std::sort(samples.begin(),samples.end());

  madai::Trace trace;
  for (std::vector< madai::Sample >::const_iterator it = samples.begin() ;
       it != samples.end(); ++it) {
    trace.Add( *it );
  }
  trace.WriteCSVOutput(
      std::cout,
      em.GetParameters(),
      em.GetScalarOutputNames() );
  return EXIT_SUCCESS;
}
