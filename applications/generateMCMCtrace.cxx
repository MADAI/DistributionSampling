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

template<class S, class T>
int findIndex(const S & v, const T & s) {
  typename S::const_iterator it = std::find(v.begin(), v.end(), s);
  if (it == v.end())
    return -1;
  return std::distance(v.begin(), it);

}

/**
   Read a experimental_results/results.dat file */
bool LoadObservations(madai::Model & m, std::istream & i) {
  // std::ifstream i("DIRECTORY/experimental_results/results.dat");
  const std::vector< std::string > & scalarOutputNames = m.GetScalarOutputNames();
  unsigned int numberOfScalarOutputs = m.GetNumberOfScalarOutputs();
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
      observedScalarCovariance[index * (1 + numberOfScalarOutputs)] = std::pow(uncertainty, 2);
      // uncertainty^2 is variance.
    }
  }
  // assume extra values are all zero.
  madai::Model::ErrorType e;
  e = m.SetObservedScalarValues(observedScalarValues);
  if (e != madai::Model::NO_ERROR) {
    std::cerr << "Error in Model::SetObservedScalarValues\n";
    return false;
  }
  e = m.SetObservedScalarCovariance(observedScalarCovariance);
  if (e != madai::Model::NO_ERROR) {
    std::cerr << "Error in Model::SetObservedScalarCovariance\n";
    return false;
  }
  return true;
}


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
  if (! LoadObservations(gpem, observations)) {
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
      gpem.GetParameters(),
      gpem.GetScalarOutputNames() );
  return EXIT_SUCCESS;
}
