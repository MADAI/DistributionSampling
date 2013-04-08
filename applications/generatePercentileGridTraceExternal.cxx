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
#include "Trace.h"

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
  //if (madai::Model::NO_ERROR != em.LoadObservations(observations)) {
  if (madai::Model::NO_ERROR != LoadObservations(&em, observations)) {
    std::cerr << "error loading observations.\n";
    em.StopProcess();
    return EXIT_FAILURE;
  }
  observations.close();

  madai::PercentileGridSampler sampler;
  sampler.SetModel( &em );
  sampler.SetNumberSamples(numberIter);
  numberIter = sampler.GetNumberSamples();

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

  trace.WriteCSVOutput(
      std::cout,
      em.GetParameters(),
      em.GetScalarOutputNames() );
  return EXIT_SUCCESS;
}
