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
#include "SimpleMetropolisHastings.h"
#include "Trace.h"

/**
 * Test case for madai::ExternalModel and
 * madai::SimpleMetropolisHastings classes.
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
  madai::ExternalModel external_model;

  // This executable does not require any arguments
  std::vector< std::string > arguments;

  external_model.StartProcess( executable, arguments );
  if ( !external_model.IsReady() ) {
    std::cerr << "Something is wrong with the external model\n";

    return EXIT_FAILURE;
  }

  madai::SimpleMetropolisHastings simple_mcmc( &external_model );

  std::vector< madai::Parameter > const * parameters
    = &(external_model.GetParameters());

  for (unsigned int i = 0; i < parameters->size(); i++) {
    simple_mcmc.ActivateParameter((*parameters)[i].m_Name);
  }

  simple_mcmc.SetOutputScalarToOptimize
    ( external_model.GetScalarOutputNames().at(0) );

  madai::Trace trace;
  unsigned int numberIter = 500;
  for (unsigned int count = 0; count < numberIter; count ++) {
    simple_mcmc.NextIteration(&trace);
  }

  trace.writeHead( std::cout,
                   external_model.GetParameters(),
                   external_model.GetScalarOutputNames() );
  trace.write( std::cout );

  return EXIT_SUCCESS;
}
