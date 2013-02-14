/*=========================================================================
 *
 *  Copyright The University of North Carolina at Chapel Hill
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

#include "ExternalModel.h"
#include "SimpleMetropolisHastingsSampler.h"
#include "Trace.h"

/**
 * Test case for madai::ExternalModel and
 * madai::SimpleMetropolisHastingsSampler classes.
 */
int main(int argc, char ** argv) {

  if (argc < 2) {
    std::cerr <<
      "Usage: " << argv[0] << " external_executable\n\n"
      "where external_executable is suitable for the\n"
      "madai::ExternalModel class.\n\n";
    return EXIT_FAILURE;
  }

  std::string executable( argv[1] );
  madai::ExternalModel externalModel;

  // This executable does not require any arguments
  std::vector< std::string > arguments;

  externalModel.StartProcess( executable, arguments );
  if ( !externalModel.IsReady() ) {
    std::cerr << "Something is wrong with the external model\n";

    return EXIT_FAILURE;
  }

  madai::SimpleMetropolisHastingsSampler simple_mcmc(&externalModel);

  std::vector< madai::Parameter > const * parameters
    = &(externalModel.GetParameters());

  for (unsigned int i = 0; i < parameters->size(); i++) {
    simple_mcmc.ActivateParameter((*parameters)[i].m_Name);
  }

  simple_mcmc.SetOutputScalarToOptimize
    ( externalModel.GetScalarOutputNames().at(0) );

  madai::Trace trace;
  unsigned int numberIter = 500;
  for (unsigned int count = 0; count < numberIter; count ++) {
    simple_mcmc.NextSample(&trace);
  }

  // Stop the external process
  externalModel.StopProcess();

  trace.writeHead( std::cout,
                   externalModel.GetParameters(),
                   externalModel.GetScalarOutputNames() );
  trace.write( std::cout );

  return EXIT_SUCCESS;
}
