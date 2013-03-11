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

#include <iostream>
#include <fstream>
#include <cmath>

#include <Eigen/Dense>
#include "Random.h"
#include "GaussianProcessEmulator.h"

const char TRAINING_FILE[] = "/tmp/EmulatorTestTraining.dat";
const char MODEL_FILE[] = "/tmp/EmulatorTestModel.dat";

inline double LogisticFunction(double x) {
  return 1.0 / (1.0 + std::exp(-x));
}
inline double sinc(double x) {
  if (x==0) return 1.0;
  x *= 3.141592653589793;
  return std::sin(x) / x;
}
inline void model(double x, double y, double & f, double & g) {
  f = sinc(std::sqrt((x * x) + (y * y) + (0.5 * x * y)));
  g = LogisticFunction(x - 0.25 * y);
}

void printTraining(
    const Eigen::MatrixXd & X,
    const Eigen::MatrixXd & Y,
    const char * filename) {
  std::ofstream o(filename); // Fixme tmpfile;
  o.precision(17);
  o << "#\n#\n#\nVERSION 1\nPARAMETERS\n2\n"
    << "X UNIFORM -1.0 1.0\nY UNIFORM -1.0 1.0\n"
    << "OUTPUTS\n2\nF\nG\nNUMBER_OF_TRAINING_POINTS\n100\n"
    << "PARAMETER_VALUES\n100 2\n" << X
    << "\nOUTPUT_VALUES\n100 2\n" << Y << "\nEND_OF_FILE\n";
}

template < typename TDerived >
inline void  LatinHypercube(
    const Eigen::MatrixBase< TDerived > & m_)
{
  madai::Random random;
  int N = 100;
  int p = 2;
  Eigen::MatrixBase< TDerived > & m
    = const_cast< Eigen::MatrixBase< TDerived > & >(m_);
  assert((N > 0) && (p > 0));
  m.resize(N,p);

  double range_over_N = 2.0 / static_cast< double >(N);
  double half_range_over_N = 0.5 * range_over_N;
  for (int i = 0; i < N; ++i) {
    double d = range_over_N * i + half_range_over_N;
    for (int j = 0; j < p; ++j) {
      m(i,j) = d;
    }
  }
  for (int j = 0; j < p; ++j) {
    for (int i = N-1; i >= 0; --i) {
      int k = random.Integer(i + 1);
      double tmp = m(k,j);
      m(k,j) = m(i,j);
      m(i,j) = tmp;
    }
  }
}

int main(int argc, char ** argv) {
  static const int N = 100;
  Eigen::MatrixXd X(N,2);
  Eigen::MatrixXd Y(N,2);
  LatinHypercube(X);
  for (int i = 0; i < N; ++i) {
    double f, g;
    model(X(i,0), X(i,1), f, g);
    Y(i,0) = f;
    Y(i,1) = g;
  }
  printTraining(X,Y,TRAINING_FILE);
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
          scale))
    return EXIT_FAILURE;

  std::ofstream ofs(MODEL_FILE);
  gpe.Write(ofs);

  std::cout.precision(17);
  double error = 0.0;

  std::vector< double > x(2);
  std::vector< double > y(2);
  for (int i = 0; i < N; ++i) {
    x[0] = X(i,0);
    x[1] = X(i,1);
    if (! gpe.GetEmulatorOutputs(x, y))
      return EXIT_FAILURE;
    error += std::abs(y[0] - Y(i,0));
    error += std::abs(y[1] - Y(i,1));
  }
  std::cout << "Sum of errors at training points: " << error << '\n';

  error = 0.0;
  double range_over_N = 2.0 / static_cast< double >(N);
  double half_range_over_N = 0.5 * range_over_N;
  for (int i = 0; i < N; ++i) {
    x[0] = range_over_N * i + half_range_over_N;
    for (int j = 0; j < N; ++j) {
      x[1] = range_over_N * j + half_range_over_N;
      gpe.GetEmulatorOutputs(x, y);
      double f, g;
      model(x[0], x[1], f, g);
      error = std::max(error, std::abs(y[0] - f));
      error = std::max(error, std::abs(y[1] - g));
    }
  }
  std::cout << "Maximum error over all space: " << error << '\n';
  return EXIT_SUCCESS;
}
