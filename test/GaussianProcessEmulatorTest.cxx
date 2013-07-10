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

#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>

#include <Eigen/Dense>
#include "GaussianProcessEmulatorTestGenerator.h"
#include "GaussianProcessEmulator.h"
#include "GaussianProcessEmulatorDirectoryFormatIO.h"
#include "Paths.h"

const char DEFAULT_MODEL_OUTPUT_DIRECTORY[] = "model_output";
const char DEFAULT_EXPERIMENTAL_RESULTS_FILE[] = "experimental_results.dat";

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


int main( int, char *[] ) {
  static const int N = 100;

  madai::Parameter param0( "param_0", -1, 1 );
  madai::Parameter param1( "param_1", -1, 1 );
  std::vector< madai::Parameter > parameters;
  parameters.push_back( param0 );
  parameters.push_back( param1 );

  GaussianProcessEmulatorTestGenerator generator( &model,2,2,N, parameters);

  std::string TempDirectory = "../Testing/Temporary/GaussianProcessEmulatorTest";
  if ( !generator.WriteDirectoryStructure(TempDirectory) ) {
    std::cerr << "Error writing directory structure\n";
    return EXIT_FAILURE;
  }

  std::string MOD = TempDirectory + madai::Paths::SEPARATOR +
    DEFAULT_MODEL_OUTPUT_DIRECTORY;
  std::string ERF = TempDirectory + madai::Paths::SEPARATOR +
    DEFAULT_EXPERIMENTAL_RESULTS_FILE;

  madai::GaussianProcessEmulator gpe;
  madai::GaussianProcessEmulatorDirectoryFormatIO directoryReader;
  if ( !directoryReader.LoadTrainingData( &gpe, MOD, TempDirectory, ERF ) ) {
    std::cerr << "Error loading from created directory structure\n";
    return EXIT_FAILURE;
  }

  if ( !gpe.PrincipalComponentDecompose() ) {
    std::cerr << "Error decomposing model data.\n";
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

  double fractionResolvingPower = 0.999;
  madai::GaussianProcessEmulator::CovarianceFunctionType covarianceFunction
      = madai::GaussianProcessEmulator::SQUARE_EXPONENTIAL_FUNCTION;
  int regressionOrder = 1;
  double defaultNugget = 1e-3;
  double amplitude = 1.0;
  double scale = 1e-2;

  if (! gpe.RetainPrincipalComponents( fractionResolvingPower ) )
    return EXIT_FAILURE;

  if (! gpe.BasicTraining(
          covarianceFunction,
          regressionOrder,
          defaultNugget,
          amplitude,
          scale))
    return EXIT_FAILURE;

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
    std::cerr << "Error while makeing cache.\n";
    return false;
  }

  std::cout.precision(17);
  double error = 0.0;

  std::vector< double > x(2,0.0);
  std::vector< double > y(2,0.0);
  for (int i = 0; i < N; ++i) {
    x[0] = generator.m_X(i,0);
    x[1] = generator.m_X(i,1);
    if (! gpe.GetEmulatorOutputs(x, y))
      return EXIT_FAILURE;
    error += std::abs(y[0] - generator.m_Y(i,0));
    error += std::abs(y[1] - generator.m_Y(i,1));
  }
  std::cout << "Sum of errors at training points: " << error << '\n';

  error = 0.0;
  std::vector< double > y2(2,0.0);
  double range_over_N = 2.0 / static_cast< double >(N);
  double half_range_over_N = 0.5 * range_over_N;
  for (int i = 0; i < N; ++i) {
    x[0] = range_over_N * i + half_range_over_N;
    for (int j = 0; j < N; ++j) {
      x[1] = range_over_N * j + half_range_over_N;
      gpe.GetEmulatorOutputs(x, y);
      model(x, y2);
      error = std::max(error, std::abs(y[0] - y2[0]));
      error = std::max(error, std::abs(y[1] - y2[1]));
    }
  }
  std::cout << "Maximum error over all space: " << error << '\n';

  std::string ThetaFileName = TempDirectory + madai::Paths::SEPARATOR + "thetas.dat";
  std::ofstream ThetaFile( ThetaFileName.c_str() );
  if(! directoryFormatIO.PrintThetas(&gpe,ThetaFile)) {
    std::cerr << "Error printing Thetas.\n";
    return EXIT_FAILURE;
  }
  ThetaFile.close();

  return EXIT_SUCCESS;
}
