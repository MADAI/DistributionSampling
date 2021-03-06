/*=========================================================================
 *
 *  Copyright 2011-2013 The University of North Carolina at Chapel Hill
 *  All rights reserved.
 *
 *  Licensed under the MADAI Software License. You may obtain a copy of
 *  this license at
 *
 *         https://madai-public.cs.unc.edu/visualization/software-license/
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "GaussianProcessEmulatorTestGenerator.h"
#include "MetropolisHastingsSampler.h"
#include "GaussianProcessEmulatedModel.h"
#include "GaussianProcessEmulatorDirectoryFormatIO.h"
#include "GaussianProcessEmulator.h"
#include "Paths.h"

const char DEFAULT_MODEL_OUTPUT_DIRECTORY[] = "model_output";
const char DEFAULTS_EXPERIMENTAL_RESULTS_FILE[] = "experimental_results.dat";

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
  static const int N = 100;

  madai::Parameter param0( "param_0", -1, 1 );
  madai::Parameter param1( "param_1", -1, 1 );
  std::vector< madai::Parameter > generatorParameters;
  generatorParameters.push_back( param0 );
  generatorParameters.push_back( param1 );

  GaussianProcessEmulatorTestGenerator generator( &model,2,2,N,
                                                  generatorParameters);

  std::string TempDirectory = "../Testing/Temporary/GaussianProcessEmulatedModelTest";
  if ( !generator.WriteDirectoryStructure(TempDirectory) ) {
    std::cerr << "Error writing directory structure\n";
    return EXIT_FAILURE;
  }

  std::string MOD = TempDirectory + madai::Paths::SEPARATOR +
    DEFAULT_MODEL_OUTPUT_DIRECTORY;
  std::string ERF = TempDirectory + madai::Paths::SEPARATOR +
    DEFAULTS_EXPERIMENTAL_RESULTS_FILE;

  madai::GaussianProcessEmulator gpe;
  madai::GaussianProcessEmulatorDirectoryFormatIO directoryReader;
  if ( !directoryReader.LoadTrainingData( &gpe, MOD, TempDirectory, ERF ) ) {
    std::cerr << "error loading from created directory structure\n";
    return EXIT_FAILURE;
  }

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

  std::string PCAFileName = TempDirectory + madai::Paths::SEPARATOR
      + madai::Paths::PCA_DECOMPOSITION_FILE;
  std::ofstream PCAFile( PCAFileName.c_str() );
  if ( !PCAFile ) {
    std::cerr << "Could not open file '" << PCAFileName << "'\n";
    return EXIT_FAILURE;
  }

  madai::GaussianProcessEmulatorDirectoryFormatIO directoryFormatIO;
  directoryFormatIO.WritePCA( &gpe, PCAFile );
  PCAFile.close();

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

  std::string EmulatorStateFileName = TempDirectory + madai::Paths::SEPARATOR
      + madai::Paths::EMULATOR_STATE_FILE;
  std::ofstream EmulatorStateFile( EmulatorStateFileName.c_str() );
  if ( !EmulatorStateFile ) {
    std::cerr << "Could not open file '" << EmulatorStateFileName << "'\n";
    return EXIT_FAILURE;
  }

  directoryFormatIO.Write( &gpe, EmulatorStateFile );
  EmulatorStateFile.close();

  if (! gpe.MakeCache()) {
    std::cerr << "Error in GaussianProcessEmulator::MakeCache().\n";
    return EXIT_FAILURE;
  }

  madai::GaussianProcessEmulatedModel gpem;
  if (gpem.SetGaussianProcessEmulator( gpe ) != madai::Model::NO_ERROR) {
    std::cerr << "Error in GaussianProcessEmulatedModel::SetGaussianProcessEmulator\n";
    return EXIT_FAILURE;
  }

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

  madai::MetropolisHastingsSampler mcmc;
  // The Model needs to be completely set up before passing to the
  // Sampler because the sampler might evaluate the Model right away.
  mcmc.SetModel( &gpem );

  mcmc.SetStepSize(0.1);

  unsigned int numberIter = 50;
  for (unsigned int count = 0; count < numberIter; count ++) {
    madai::Sample sample = mcmc.NextSample();
    std::cout << sample << "\n";

    // Emulated value
    std::vector< double > emulatedOutput;
    gpe.GetEmulatorOutputs( sample.m_ParameterValues, emulatedOutput );
    std::cout << "\n";

    for ( size_t i = 0; i < emulatedOutput.size(); ++i ) {
      if ( std::fabs( emulatedOutput[i] - sample.m_OutputValues[i] ) > 1e-5 ) {
        std::cerr << "Difference found in GaussianProcessEmulatedModel result "
                  << "and direct GaussianProcessEmulator result.\n";
        return EXIT_FAILURE;
      }
    }
  }

  gpem.SetUseModelCovarianceToCalulateLogLikelihood(false);

  for (unsigned int count = 0; count < numberIter; count ++) {
    madai::Sample sample = mcmc.NextSample();
    std::cout << sample << "\n";

    // Emulated value
    std::vector< double > emulatedOutput;
    gpe.GetEmulatorOutputs( sample.m_ParameterValues, emulatedOutput );

    std::cout << "Direct emulator result: ";
    for ( size_t i = 0; i < emulatedOutput.size(); ++i ) {
      std::cout << emulatedOutput[i] << ", ";
    }
    std::cout << "\n";

    for ( size_t i = 0; i < emulatedOutput.size(); ++i ) {
      if ( std::fabs( emulatedOutput[i] - sample.m_OutputValues[i] ) > 1e-5 ) {
        std::cerr << "Difference found in GaussianProcessEmulatedModel result "
                  << "and direct GaussianProcessEmulator result.\n";
        return EXIT_FAILURE;
      }
    }
  }

  return EXIT_SUCCESS;
}
