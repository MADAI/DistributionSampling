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
#include "MetropolisHastingsSampler.h"
#include "GaussianProcessEmulatedModel.h"
#include "GaussianProcessEmulatorSingleFileReader.h"
#include "GaussianProcessEmulatorSingleFileWriter.h"
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
 * madai::MetropolisHastingsSampler classes.
 */
int main( int, char *[] ) {

  const char TRAINING_FILE[] = "/tmp/GaussianProcessEmulatorTestTraining.dat";
  const char MODEL_FILE[] = "/tmp/GaussianProcessEmulatorTestModel.dat";
  static const int N = 100;

  madai::Parameter param0( "param_0", -1, 1 );
  madai::Parameter param1( "param_1", -1, 1 );
  std::vector< madai::Parameter > generatorParameters;
  generatorParameters.push_back( param0 );
  generatorParameters.push_back( param1 );

  GaussianProcessEmulatorTestGenerator generator( &model,2,2,N,
                                                  generatorParameters);

  std::ofstream out(TRAINING_FILE);
  generator.WriteTrainingFile(dynamic_cast<std::ostream &>(out));
  out.close();

  madai::GaussianProcessEmulator gpe;
  std::ifstream ifs(TRAINING_FILE);
  madai::GaussianProcessEmulatorSingleFileReader singleFileReader;
  singleFileReader.LoadTrainingData(&gpe,ifs);
  double fractionResolvingPower = 0.999;
  madai::GaussianProcessEmulator::CovarianceFunctionType covarianceFunction
    = madai::GaussianProcessEmulator::SQUARE_EXPONENTIAL_FUNCTION;
  int regressionOrder = 1;
  double defaultNugget = 1e-3;
  double amplitude = 1.0;
  double scale = 1e-2;

  if ( !gpe.PrincipalComponentDecompose() ) {
    std::cerr << "Error in GaussianProcessEmulator::PrincipalComponentDecompose\n";
    return EXIT_FAILURE;
  }

  if ( !gpe.RetainPrincipalComponents( fractionResolvingPower ) ) {
    std::cerr << "Error in GaussianProcessEmulator::RetainPrincipalComponents\n";
    return EXIT_FAILURE;
  }

  if (! gpe.BasicTraining(
          covarianceFunction,
          regressionOrder,
          defaultNugget,
          amplitude,
          scale)) {
    std::cerr << "Error in GaussianProcessEmulator::BasicTraining";
    return EXIT_FAILURE;
  }
  out.open(MODEL_FILE);
  madai::GaussianProcessEmulatorSingleFileWriter singleFileWriter;
  singleFileWriter.Write(&gpe,out);
  out.close();

  if (! gpe.MakeCache()) {
    std::cerr << "Error in GaussianProcessEmulator::MakeCache().\n";
    return EXIT_FAILURE;
  }

  madai::GaussianProcessEmulatedModel gpem;
  if (gpem.SetGaussianProcessEmulator( gpe ) != madai::Model::NO_ERROR) {
    std::cerr << "Error in GaussianProcessEmulatedModel::SetGaussianProcessEmulator\n";
    return EXIT_FAILURE;
  }

  madai::MetropolisHastingsSampler mcmc;
  mcmc.SetModel( &gpem );

  mcmc.SetStepSize(0.1);

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
