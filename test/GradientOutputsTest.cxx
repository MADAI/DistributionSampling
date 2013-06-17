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
#include <iostream>
#include <string>
#include <vector>

#include "Defaults.h"
#include "GaussianProcessEmulatorTestGenerator.h"
#include "GaussianProcessEmulatedModel.h"
#include "GaussianProcessEmulatorDirectoryFormatIO.h"
#include "GaussianProcessEmulator.h"
#include "Random.h"
#include "Trace.h"
#include "Paths.h"

inline double LogisticFunction(double x) {
  return 1.0 / ( 1.0 + std::exp( - x ) );
}
inline double sinc(double x) {
  if (x==0) return 1.0;
  x *= 3.141592653589793;
  return (std::sin(x) / x);
}
void model(const std::vector< double > & params, std::vector< double > & out) {
  const double & x = params[0];
  const double & y = params[1];
  out.at(0) = sinc(std::sqrt( x*x + y*y + 0.5*x*y ));
  out.at(1) = LogisticFunction( x - 0.25 * y );
}

/**
 * Test to see if the analytic and numeric gradients
 * from the emulator are consistent.
 */
int main( int, char*[] ) {
  static const int N = 100;

  madai::Parameter param0 ( "param_0", -1, 1 );
  madai::Parameter param1 ( "param_1", -1, 1 );
  std::vector< madai::Parameter > generatorParameters;
  generatorParameters.push_back( param0 );
  generatorParameters.push_back( param1 );

  GaussianProcessEmulatorTestGenerator generator( &model, 2, 2, N,
                                                  generatorParameters);

  std::string TempDirectory = "/tmp/";
  if ( !generator.WriteDirectoryStructure( TempDirectory ) ) {
    std::cerr << "Error writing directory structure.\n";
    return EXIT_FAILURE;
  }

  std::string MOD = TempDirectory + madai::Paths::SEPARATOR +
    madai::Defaults::MODEL_OUTPUT_DIRECTORY;
  std::string ERD = TempDirectory + madai::Paths::SEPARATOR +
    madai::Defaults::EXPERIMENTAL_RESULTS_FILE;

  madai::GaussianProcessEmulator gpe;
  madai::GaussianProcessEmulatorDirectoryFormatIO directoryReader;
  if ( !directoryReader.LoadTrainingData( &gpe, MOD, TempDirectory, ERD ) ) {
    std::cerr << "Error loading from created directory structure.\n";
    return EXIT_FAILURE;
  }

  double fractionResolvingPower = 0.999;
  madai::GaussianProcessEmulator::CovarianceFunctionType CovarianceFunction
    = madai::GaussianProcessEmulator::SQUARE_EXPONENTIAL_FUNCTION;
  int regressionOrder = 1;
  double defaultNugget = 1e-3;
  double amplitude = 1.0;
  double scale = 1e-2;

  if ( !gpe.PrincipalComponentDecompose() ) {
    std::cerr << "Error in GaussianProcessEmulator::PrincipalComponentDecompose\n";
    return EXIT_FAILURE;
  }

  std::string PCAFileName = TempDirectory + madai::Paths::SEPARATOR +
                            madai::Paths::PCA_DECOMPOSITION_FILE;
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

  if ( !gpe.BasicTraining( CovarianceFunction,
                           regressionOrder,
                           defaultNugget,
                           amplitude,
                           scale ) ) {
    std::cerr << "Error in GaussianProcessEmulator::BasicTraining";
    return EXIT_FAILURE;
  }

  std::string EmulatorStateFileName = TempDirectory + madai::Paths::SEPARATOR +
                                      madai::Paths::EMULATOR_STATE_FILE;
  std::ofstream EmulatorStateFile( EmulatorStateFileName.c_str() );
  if ( !EmulatorStateFile ) {
    std::cerr << "Could not open file '" << EmulatorStateFileName << "'\n";
    return EXIT_FAILURE;
  }

  directoryFormatIO.Write( &gpe, EmulatorStateFile );
  EmulatorStateFile.close();

  if ( !gpe.MakeCache() ) {
    std::cerr << "Error in GaussianProcessEmulator::MakeCache().\n";
    return EXIT_FAILURE;
  }

  madai::GaussianProcessEmulatedModel gpem;
  if ( gpem.SetGaussianProcessEmulator( gpe ) != madai::Model::NO_ERROR ) {
    std::cerr << "Error in GaussianProcessEmulatedModel::SetGaussianProcessEmulator.\n";
    return EXIT_FAILURE;
  }

  // Check gradients at 100 random places in parameter space
  assert ( 2 == gpem.GetNumberOfParameters() );
  int p = gpem.GetNumberOfParameters();
  std::vector< bool > activeParameters;
  for ( int i = 0; i < p; i++ )
    activeParameters.push_back(true);
  unsigned int NPoints = 500;
  madai::Random RandomNumberGenerator;
  int Count = 0;
  for ( unsigned int i = 0; i < NPoints; i++ ) {
    // Choose random point in parameter space
    std::vector< double > currentParameters(p,0.0);
    for ( int j = 0; j < p; j++ ) {
      const madai::Distribution * priorDist
          = generatorParameters[j].GetPriorDistribution();
      currentParameters[j] = priorDist->GetSample(RandomNumberGenerator);
    }

    // Get the analytic gradient
    gpem.SetUseModelCovarianceToCalulateLogLikelihood(false);
    std::vector< double > grad;
    std::vector< double > scalars;
    if ( gpem.GetScalarAndGradientOutputs( currentParameters,
             activeParameters, scalars, grad ) != madai::Model::NO_ERROR ) {
      std::cerr << "Error in Model::GetAnalyticGradientOfLogLikelihood.\n";
      return EXIT_FAILURE;
    }

    // Get the numeric gradient
    std::vector< double > grad2;
    if ( gpem.Model::GetScalarAndGradientOutputs(currentParameters,
             activeParameters, scalars, grad2) != madai::Model::NO_ERROR ) {
      std::cerr << "Error in Model:GetScalarAndGradientOutputs.\n";
      return EXIT_FAILURE;
    }

    // Compare gradients
    std::vector< double > diff(p,0.0);
    double AnalyticNormSquared = 0;
    double DiffNormSquared = 0;
    for ( int j = 0; j < p; j++ ) {
      diff[j] = grad[j] - grad2[j];
      AnalyticNormSquared += grad[j] * grad[j];
      DiffNormSquared += diff[j] * diff[j];
    }
    double ratio = std::sqrt(DiffNormSquared/AnalyticNormSquared);
    if ( ratio < 0.01 ) {
      Count++;
    }
  }
  double percentConsistent = double(Count)/double(NPoints);
  if ( percentConsistent < 0.99 ) {
    std::cerr << "Number of points with consistent gradients too small.\n";
    std::cerr << "Percent Consistent: " << percentConsistent << "\n";
    return EXIT_FAILURE;
  } else {
    std::cerr << "Test passed with gradient consistency of " << percentConsistent << "\n";
    return EXIT_SUCCESS;
  }
}
