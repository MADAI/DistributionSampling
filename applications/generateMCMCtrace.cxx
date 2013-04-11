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
#include "GaussianProcessEmulatedModel.h"
#include "Trace.h"


/**
   \fixme document
 */
int main(int argc, char ** argv) {

  if (argc < 4) {
    std::cerr << "Useage:\n  "
      "generateMCMCTrace emulatorFile observationsFile N\n\n";
    return EXIT_FAILURE; //\fixme useage
  }
  const char * emulatorFile = argv[1];
  const char * observationsFile = argv[2];
  int numberIter = atoi(argv[3]);

  madai::GaussianProcessEmulatedModel gpem;
  if (gpem.LoadConfigurationFile( emulatorFile ) != madai::Model::NO_ERROR) {
    std::cerr << "Error in GaussianProcessEmulatedModel::LoadConfigurationFile\n";
    return EXIT_FAILURE;
  }

  gpem.SetUseModelCovarianceToCalulateLogLikelihood(false);

  std::ifstream observations(observationsFile);
  if (madai::Model::NO_ERROR != gpem.LoadObservations(observations)) {
    std::cerr << "error loading observations.\n";
    return EXIT_FAILURE;
  }
  observations.close();

  madai::MetropolisHastingsSampler mcmc;
  mcmc.SetModel( &gpem );
  mcmc.SetStepSize(0.1);

  std::vector< madai::Parameter > const & parameters
    = gpem.GetParameters();

  int t = gpem.GetNumberOfScalarOutputs();

  int step = numberIter / 100, percent = 0;
  if (step < 1)
    step = 1; // avoid div-by-zero error;
  std::vector< madai::Sample> samples;
  for (int count = 0; count < numberIter; count ++) {
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
  trace.WriteCSVOutput(
      std::cout,
      gpem.GetParameters(),
      gpem.GetScalarOutputNames() );
  return EXIT_SUCCESS;
}
