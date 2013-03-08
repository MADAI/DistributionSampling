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

  madai::SimpleMetropolisHastingsSampler simple_mcmc;
  simple_mcmc.SetModel( &externalModel );

  simple_mcmc.SetStepSize(0.1);

  std::vector< madai::Parameter > const & parameters
    = externalModel.GetParameters();

  int t = externalModel.GetNumberOfScalarOutputs();
  std::vector< double > observedScalarValues;
  for(int i = 0; i < t; ++i)
    observedScalarValues.push_back(0.2);
  externalModel.SetObservedScalarValues(observedScalarValues);
  std::vector< double > observedScalarCovariance(t * t, 0.0);
  for(int i = 0; i < t; ++i)
    observedScalarCovariance[i + (t * i)] = 0.05;
  externalModel.SetObservedScalarCovariance(observedScalarCovariance);

  madai::Trace trace;
  unsigned int numberIter = 500;
  for (unsigned int count = 0; count < numberIter; count ++) {
    madai::Sample sample = simple_mcmc.NextSample();
    trace.Add( sample );
  }

  // Stop the external process
  externalModel.StopProcess();

  trace.WriteCSVOutput( std::cout,
                   externalModel.GetParameters(),
                   externalModel.GetScalarOutputNames() );
  //trace.WriteData( std::cout );

  return EXIT_SUCCESS;
}
