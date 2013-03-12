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

#include "GaussianProcessEmulatorTestGenerator.h"
#include "SimpleMetropolisHastingsSampler.h"
#include "GaussianProcessEmulatedModel.h"
#include "Trace.h"

inline double LogisticFunction(double x) {
  return 1.0 / (1.0 + std::exp(-x));
}
inline double sinc(double x) {
  if (x==0) return 1.0;
  x *= 3.141592653589793;
  return std::sin(x) / x;
}
void model(const std::vector< double > & params, std::vector< double > & out) {
  const double & x = params[0];
  const double & y = params[1];
  out.at(0) = sinc(std::sqrt((x * x) + (y * y) + (0.5 * x * y)));
  out.at(1) = LogisticFunction(x - 0.25 * y);
}

/**
 * Test case for madai::GaussianProcessEmulatedModel and
 * madai::SimpleMetropolisHastingsSampler classes.
 */
int main(int argc, char ** argv) {

  const char TRAINING_FILE[] = "/tmp/GaussianProcessEmulatorTestTraining.dat";
  const char MODEL_FILE[] = "/tmp/GaussianProcessEmulatorTestModel.dat";
  static const int N = 100;
  double pmin[2] = {-1, -1};
  double pmax[2] = {1, 1};
  std::vector< double > parameterMinima(pmin, pmin + 2);
  std::vector< double > parameterMaxima(pmax, pmax + 2);

  GaussianProcessEmulatorTestGenerator generator(
      &model,2,2,N, parameterMinima,parameterMaxima);

  std::ofstream out(TRAINING_FILE);
  generator.WriteTrainingFile(dynamic_cast<std::ostream &>(out));
  out.close();

  madai::GaussianProcessEmulator gpe;
  std::ifstream ifs(TRAINING_FILE);
  gpe.LoadTrainingData(ifs);
  double fractionResolvingPower = 0.999;
  madai::GaussianProcessEmulator::CovarianceFunction covarianceFunction
    = madai::GaussianProcessEmulator::SQUARE_EXP_FN;
  int regressionOrder = 1;
  double defaultNugget = 1e-3;
  double amplitude = 1.0;
  double scale = 1e-2;

  if (! gpe.BasicTraining(
          fractionResolvingPower,
          covarianceFunction,
          regressionOrder,
          defaultNugget,
          amplitude,
          scale)) {
    std::cerr << "Error in GaussianProcessEmulator::BasicTraining";
    return EXIT_FAILURE;
  }
  out.open(MODEL_FILE);
  gpe.Write(out);
  out.close();

  madai::GaussianProcessEmulatedModel gpem;
  if (gpem.LoadConfigurationFile( MODEL_FILE ) != madai::Model::NO_ERROR) {
    std::cerr << "Error in GaussianProcessEmulatedModel::LoadConfigurationFile";
    return EXIT_FAILURE;
  }

  madai::SimpleMetropolisHastingsSampler mcmc;
  mcmc.SetModel( &gpem );

  mcmc.SetStepSize(0.1);

  std::vector< madai::Parameter > const & parameters
    = gpem.GetParameters();

  assert (2 == gpem.GetNumberOfScalarOutputs());
  int t = gpem.GetNumberOfScalarOutputs();
  std::vector< double > observedScalarValues;
  for(int i = 0; i < t; ++i)
    observedScalarValues.push_back(0.2);
  gpem.SetObservedScalarValues(observedScalarValues);
  std::vector< double > observedScalarCovariance(t * t, 0.0);
  for(int i = 0; i < t; ++i)
    observedScalarCovariance[i + (t * i)] = 0.05;
  gpem.SetObservedScalarCovariance(observedScalarCovariance);

  madai::Trace trace;
  unsigned int numberIter = 500;
  for (unsigned int count = 0; count < numberIter; count ++) {
    madai::Sample sample = mcmc.NextSample();
    trace.Add( sample );
  }

  gpem.SetUseModelCovarianceToCalulateLogLikelihood(false);

  for (unsigned int count = 0; count < numberIter; count ++) {
    madai::Sample sample = mcmc.NextSample();
    trace.Add( sample );
  }


  trace.WriteCSVOutput( std::cout,
                   gpem.GetParameters(),
                   gpem.GetScalarOutputNames() );
  //trace.WriteData( std::cout );

  return EXIT_SUCCESS;
}
