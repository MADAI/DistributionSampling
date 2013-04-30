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

/*
 * GaussianProcessEmulator Class
 *
 * \author Hal Canary <cs.unc.edu/~hal/>
 * \author Christopher Coleman-Smith <cec24@phy.duke.edu>
 * \author Cory Quammen <cs.unc.edu/~cquammen>
 */

#include <cmath>        // std::exp std::amp
#include <limits>       // std::numeric_limits
#include <fstream>      // std::ofstream std::ifstream
#include "GaussianProcessEmulator.h"
#include "UniformDistribution.h"
#include "GaussianDistribution.h"
#include "Paths.h"

#include "madaisys/Directory.hxx"


namespace {
using namespace madai;

inline int NumberRegressionFunctions(
    int regressionOrder,
    int numberParameters) {
  return 1 + (regressionOrder * numberParameters);
}

inline int NumberThetas(
    GaussianProcessEmulator::CovarianceFunctionType cf,
    int numberParameters) {
  switch(cf) {
  case GaussianProcessEmulator::SQUARE_EXPONENTIAL_FUNCTION:
    return numberParameters + 2;
  case GaussianProcessEmulator::POWER_EXPONENTIAL_FUNCTION:
    return numberParameters + 3;
  case GaussianProcessEmulator::MATERN_32_FUNCTION:
    return numberParameters + 2;
  case GaussianProcessEmulator::MATERN_52_FUNCTION:
    return numberParameters + 2;
  case GaussianProcessEmulator::UNKNOWN_FUNCTION:
    //fall through
  default:
    return -1;
  }
}


template < typename TDerived >
inline void MakeHMatrix(
    const Eigen::MatrixBase< TDerived > & X,
    const Eigen::MatrixBase< TDerived > & H_,
    int regressionOrder)
{
  int p = X.cols(), N = X.rows();
  int numberRegressionFunctions = 1 + (regressionOrder * p);
  Eigen::MatrixBase< TDerived > & H
    = const_cast< Eigen::MatrixBase< TDerived > & >(H_);
  H.derived().resize(N, numberRegressionFunctions);
  H.block(0,0,N,1) = Eigen::MatrixXd::Constant(N,1, 1.0);
  if (regressionOrder > 0)
    H.block(0,1,N,p) = X;
  for (int i = 1; i < regressionOrder; ++i) {
    H.block(0,1+(i*p),N,p) = H.block(0,1+((i-1)*p),N,p).cwiseProduct(X);
  }
}
template < typename TDerived >
inline void MakeHVector(
    const Eigen::MatrixBase< TDerived > & point,
    const Eigen::MatrixBase< TDerived > & hvec_,
    int regressionOrder)
{
  int p = point.size();
  int numberRegressionFunctions = 1 + (regressionOrder * p);
  Eigen::MatrixBase< TDerived > & hvec
    = const_cast< Eigen::MatrixBase< TDerived > & >(hvec_);
  hvec.derived().resize(numberRegressionFunctions,1);
  hvec(0) = 1.0;
  if (regressionOrder > 0)
    hvec.segment(1,p) = point;
  for (int i = 1; i < regressionOrder; ++i) {
    hvec.segment(1+(i*p),p)
      = hvec.segment(1+((i-1)*p),p).cwiseProduct(point);
  }
}

} // anonymous namespace

namespace madai {

double GaussianProcessEmulator::SingleModel::CovarianceCalc(
    const Eigen::VectorXd & v1, const Eigen::VectorXd & v2) const
{
  static const double EPSILON = 1e-10;
  int p = m_Parent->m_NumberParameters;
  int offset;
  switch(m_CovarianceFunction) {
  case POWER_EXPONENTIAL_FUNCTION:
    offset = 3;
    break;
  case SQUARE_EXPONENTIAL_FUNCTION:
  case MATERN_32_FUNCTION:
  case MATERN_52_FUNCTION:
    offset = 2;
    break;
  default:
    assert(false);
    return 0.0; // we should throw an exception.
  }
  assert(m_Thetas.size() == (p + offset));
  const double & amplitude = m_Thetas(0);
  const double & nugget = m_Thetas(1);
  double nug = 0.0;

  double distanceSquared = 0.0;
  for (int i = 0; i < p; i++) {
    double d = v1(i) - v2(i);
    double l = m_Thetas(i + offset);
    distanceSquared += std::pow( (d / l), 2);
  }
  if (distanceSquared < EPSILON) {
    nug = nugget;
  }

  switch(m_CovarianceFunction) {
  case POWER_EXPONENTIAL_FUNCTION:
    {
      const double & power = m_Thetas(2);
      assert ((power > 0.0) && (power <= 2.0));
      return nug + amplitude * std::exp(
          -0.5 * std::pow(distanceSquared,0.5 * power));
    }
  case SQUARE_EXPONENTIAL_FUNCTION:
    {
      return nug + amplitude * std::exp( -0.5 * distanceSquared);
    }
  case MATERN_32_FUNCTION:
    {
      static const double ROOT3 = 1.7320508075688772;
      double distance = std::sqrt(distanceSquared);
      return nug + amplitude * (1 + ROOT3 * distance)
        * std::exp(- ROOT3 * distance);
    }
  case MATERN_52_FUNCTION:
    {
      static const double ROOT5 = 2.23606797749979;
      double distance = std::sqrt(distanceSquared);
      return nug + amplitude *
        (1 + (ROOT5 * distance) + ((5.0 / 3.0) * distanceSquared)) *
        std:: exp(- ROOT5 * distance);
    }
  default:
    assert(false);
    return 0.0;
  }
}


GaussianProcessEmulator::StatusType
GaussianProcessEmulator::CheckStatus() {
  m_Status = UNINITIALIZED;
  if (m_NumberTrainingPoints < 1) {
    return m_Status;
  }
  if (m_NumberParameters < 1) {
    return m_Status;
  }
  if (m_NumberOutputs < 1) {
    return m_Status;
  }
  if (static_cast<int>(m_Parameters.size()) != m_NumberParameters) {
    return m_Status;
  }
  if (static_cast<int>(m_OutputNames.size()) != m_NumberOutputs) {
    return m_Status;
  }
  if (m_ParameterValues.rows() != m_NumberTrainingPoints) {
    return m_Status;
  }
  if (m_OutputValues.rows() != m_NumberTrainingPoints) {
    return m_Status;
  }
  if (m_ParameterValues.cols() != m_NumberParameters) {
    return m_Status;
  }
  if (m_OutputValues.cols() != m_NumberOutputs) {
    return m_Status;
  }
  if(m_OutputUncertaintyScales.size() != m_NumberOutputs) {
    m_OutputUncertaintyScales
      = Eigen::VectorXd::Constant(m_NumberOutputs,1.0);
  }
  if(m_ObservedOutputValues.size() != m_NumberOutputs) {
    m_ObservedOutputValues
      = Eigen::VectorXd::Constant(m_NumberOutputs,0.0);
  }
  m_Status = UNTRAINED;
  if(m_NumberPCAOutputs < 1)
    return m_Status;
  if(m_OutputMeans.size() != m_NumberOutputs)
    return m_Status;
  if(m_PCAEigenvalues.size() != m_NumberOutputs)
    return m_Status;
  if(m_PCAEigenvectors.rows() != m_NumberOutputs)
    return m_Status;
  if(m_PCAEigenvectors.cols() != m_NumberOutputs)
    return m_Status;
  if(m_RetainedPCAEigenvalues.size() != m_NumberPCAOutputs)
    return m_Status;
  if(m_RetainedPCAEigenvectors.rows() != m_NumberOutputs)
    return m_Status;
  if(m_RetainedPCAEigenvectors.cols() != m_NumberPCAOutputs)
    return m_Status;
  if (static_cast<int>(m_PCADecomposedModels.size()) != m_NumberPCAOutputs)
    return m_Status;
  for (int i = 0; i < m_NumberPCAOutputs; ++i) {
    SingleModel & m = m_PCADecomposedModels[i];
    if (m.m_Parent == NULL)
      return m_Status;
    if (m.m_CovarianceFunction == UNKNOWN_FUNCTION)
      return m_Status;
    if (m.m_RegressionOrder < 0)
      return m_Status;
    if (m.m_ZValues.size() != m_NumberTrainingPoints)
      return m_Status;
    if (m.m_Thetas.size() !=
        NumberThetas(m.m_CovarianceFunction, m_NumberParameters))
      return m_Status;
  }
  m_Status = UNCACHED;
  for (int i = 0; i < m_NumberPCAOutputs; ++i) {
    SingleModel & m = m_PCADecomposedModels[i];
    int F = NumberRegressionFunctions(m.m_RegressionOrder,m_NumberParameters);
    if (m.m_CInverse.rows() != m_NumberTrainingPoints)
      return m_Status;
    if (m.m_CInverse.cols() != m_NumberTrainingPoints)
      return m_Status;
    if (m.m_RegressionMatrix1.rows() != F)
      return m_Status;
    if (m.m_RegressionMatrix1.cols() != F)
      return m_Status;
    if (m.m_RegressionMatrix2.rows() != F)
      return m_Status;
    if (m.m_RegressionMatrix2.cols() != m_NumberTrainingPoints)
      return m_Status;
    if (m.m_BetaVector.size() != F)
      return m_Status;
    if (m.m_GammaVector.size() != m_NumberTrainingPoints)
      return m_Status;
  }
  m_Status = READY;
  return m_Status;
}

void GaussianProcessEmulator::GetOutputUncertaintyScales(
    std::vector< double > & x)
{
  if(m_OutputUncertaintyScales.size() != m_NumberOutputs) {
    x.assign(m_NumberOutputs,0.0);
  } else {
    x.resize(m_NumberOutputs);
    for (int i = 0; i < m_NumberOutputs; ++i)
      x[i] = m_OutputUncertaintyScales(i);
  }
}

void GaussianProcessEmulator::GetOutputObservedValues(
    std::vector< double > & x)
{
  if(m_ObservedOutputValues.size() != m_NumberOutputs) {
    x.assign(m_NumberOutputs,0.0);
  } else {
    x.resize(m_NumberOutputs);
    for (int i = 0; i < m_NumberOutputs; ++i)
      x[i] = m_ObservedOutputValues(i);
  }
}

/**
   Set default values to uninitialized values. */
GaussianProcessEmulator::SingleModel::SingleModel() :
  m_Parent(NULL),
  m_CovarianceFunction(UNKNOWN_FUNCTION),
  m_RegressionOrder(-1)
{ }


/**
   Use m_OutputUncertaintyScales, m_OutputValues, m_OutputMeans, and
   m_RetainedPCAEigenvectors to determine m_PCADecomposedModels[i].m_ZValues; */
bool GaussianProcessEmulator::BuildZVectors() {
  if (m_PCADecomposedModels.size() != static_cast< size_t >( m_NumberPCAOutputs ) ) {
    std::cout << "Error [m_PCADecomposedModels.size() == "
              << m_PCADecomposedModels.size() << " != m_NumberPCAOutputs == "
              << m_NumberPCAOutputs << "]\n";
    return false;
  }
  Eigen::MatrixXd Y_standardized(m_NumberTrainingPoints, m_NumberOutputs);
  for (int i = 0; i < m_NumberOutputs; ++i) {
    double scale = 1.0 / m_OutputUncertaintyScales(i);
    for (int j = 0; j < m_NumberTrainingPoints; ++j) {
      Y_standardized(j, i)
        = scale * (m_OutputValues(j,i) - m_OutputMeans(i));
    }
  }
  Eigen::MatrixXd zMatrix = Y_standardized * m_RetainedPCAEigenvectors;
  for (int i = 0; i < m_NumberPCAOutputs; ++i) {
    GaussianProcessEmulator::SingleModel & m = m_PCADecomposedModels[i];
    m.m_ZValues = zMatrix.col(i);
  }
  return true;
}


/**
   Once Load(), Train(), or BasicTraining() finishes, calculate and
   cache some data to make calling GetEmulatorOutputsAndCovariance()
   faster. */
bool GaussianProcessEmulator::MakeCache() {
  if ((m_Status != READY) && (m_Status != UNCACHED))
    return false;
  assert(m_NumberPCAOutputs == static_cast<int>(m_PCADecomposedModels.size()));
  bool errorflag = false;
#if defined( OPENMP_FOUND )
  #pragma omp parallel for
#endif // OPENMP_FOUND
  for (int i = 0; i < m_NumberPCAOutputs; ++i) {
    if (! m_PCADecomposedModels[i].MakeCache())
      errorflag = true;
  }
  if (errorflag)
    return false;
  m_Status = READY;
  return true;
}

bool GaussianProcessEmulator::SingleModel::MakeCache() {
  int N = m_Parent->m_NumberTrainingPoints;
  int p = m_Parent->m_NumberParameters;
  int F = NumberRegressionFunctions(m_RegressionOrder, p);
  const Eigen::MatrixXd & X = m_Parent->m_ParameterValues;

  // allocate members
  m_CInverse.resize(N, N);
  m_BetaVector.resize(F);
  m_GammaVector.resize(N);
  m_RegressionMatrix1.resize(N, F);
  m_RegressionMatrix2.resize(F, N);

  // local matrices that appear in formulae.
  Eigen::MatrixXd HMatrix(N, F);
  Eigen::MatrixXd CMatrix(N, N);

  // CALCULATE HMatrix
  MakeHMatrix(X, HMatrix, m_RegressionOrder);

  // CALCULATE CMatrix
  // CMatrix is the covariance matrix of the design with itself.
  for (int j = 0; j < N; ++j) {
    Eigen::VectorXd rowj = X.row(j);
    for (int k = j; k < N; ++k) {
      Eigen::VectorXd rowk = X.row(k);
      CMatrix(j,k) = this->CovarianceCalc(rowj, rowk);
      if (j != k)
        CMatrix(k,j) = CMatrix(j,k);
    }
  }

  // CALCULATE CACHE VARIABLES
  m_CInverse = CMatrix.ldlt().solve(Eigen::MatrixXd::Identity(N,N));

  m_RegressionMatrix1 = (HMatrix.transpose() * m_CInverse *
                        HMatrix).ldlt().solve(Eigen::MatrixXd::Identity(F,F));

  m_RegressionMatrix2 = (m_CInverse * HMatrix).transpose();

  m_BetaVector = (m_RegressionMatrix1 * HMatrix.transpose() *
                  m_CInverse * m_ZValues);
  m_GammaVector = m_CInverse * (m_ZValues - (HMatrix * m_BetaVector));
  return true;
}

/**
   Default to uninitialized state. */
GaussianProcessEmulator::GaussianProcessEmulator() :
  m_Status(UNINITIALIZED),
  m_NumberParameters(0),
  m_NumberOutputs(0),
  m_NumberTrainingPoints(0),
  m_NumberPCAOutputs(0)
{ }


/**
   This takes an GPEM and trains it. \returns true on sucess. */
bool GaussianProcessEmulator::SingleModel::Train(
    GaussianProcessEmulator::CovarianceFunctionType covarianceFunction,
    int regressionOrder)
{
  if (regressionOrder < 0) {
    /* \todo error message to stderr */
    return false;
  }
  if (regressionOrder > 3) {
    /* \todo error message to stderr */
    return false;
  }
  m_CovarianceFunction = covarianceFunction;
  m_RegressionOrder = regressionOrder;
  int numberThetas
    = NumberThetas(covarianceFunction, m_Parent->m_NumberParameters);
  m_Thetas.resize(numberThetas);
  std::cerr << "Sorry, but this function is not yet implemented.\n";
  // FIXME
  return false;
}


/**
   This takes an GPEM and trains it. \returns true on sucess. */
bool GaussianProcessEmulator::Train(
    GaussianProcessEmulator::CovarianceFunctionType covarianceFunction,
    int regressionOrder)
{
  if (this->CheckStatus() == UNINITIALIZED)
    return false;
  m_Status = UNTRAINED;
  int t = m_NumberPCAOutputs;
  m_PCADecomposedModels.resize( t );
  for (int i = 0; i < t; ++i) {
    m_PCADecomposedModels[i].m_Parent = this;
  }
  for (int i = 0; i < t; ++i) {
    if (! m_PCADecomposedModels[i].Train(covarianceFunction,regressionOrder))
      return false;
  }
  if (! this->MakeCache()) {
    std::cerr << "FIXME need error message\n";
    return false;
  }
  return true;
}

/**
   This takes an GPEM and trains it. \returns true on sucess. */
bool GaussianProcessEmulator::BasicTraining(
    CovarianceFunctionType covarianceFunction,
    int regressionOrder,
    double defaultNugget,
    double amplitude,
    double scale)
{
  if (this->CheckStatus() == UNINITIALIZED)
    return false;
  m_Status = UNTRAINED;
  int t = m_NumberPCAOutputs;
  m_PCADecomposedModels.resize( t );
  for (int i = 0; i < t; ++i) {
    m_PCADecomposedModels[i].m_Parent = this;
  }
  for (int i = 0; i < t; ++i) {
    if (! m_PCADecomposedModels[i].BasicTraining(covarianceFunction,
            regressionOrder, defaultNugget, amplitude, scale))
      return false;
  }
  m_Status = UNCACHED;
  // if (! this->MakeCache()) {
  //   std::cerr << "FIXME need error message\n";
  //   return false;
  // }
  return true;
}
/**
   Sets default values for all of the hyperparameters. \returns
   true on success. */
bool GaussianProcessEmulator::SingleModel::BasicTraining(
    CovarianceFunctionType covarianceFunction,
    int regressionOrder,
    double defaultNugget,
    double amplitude,
    double scale) {
  m_CovarianceFunction = covarianceFunction;
  m_RegressionOrder = regressionOrder;
  int p = m_Parent->m_NumberParameters;
  int numberOfThetas = NumberThetas(m_CovarianceFunction, p);
  int offset = numberOfThetas - p;
  m_Thetas.resize(numberOfThetas);

  switch(m_CovarianceFunction) {

  case GaussianProcessEmulator::SQUARE_EXPONENTIAL_FUNCTION:
    m_Thetas.resize(2 + p);
    m_Thetas(0) = amplitude;
    m_Thetas(1) = defaultNugget;
    for (int j = 0; j < p; ++j) {
      const madai::Parameter & param = m_Parent->m_Parameters[j];
      const madai::Distribution * priordist = param.m_PriorDistribution;
      m_Thetas(2+j) = scale * std::abs(priordist->GetPercentile(0.75)
                                     - priordist->GetPercentile(0.25));
    }
    break;
  case GaussianProcessEmulator::POWER_EXPONENTIAL_FUNCTION:
    m_Thetas.resize(3 + p);
    m_Thetas(0) = amplitude;
    m_Thetas(1) = defaultNugget;
    m_Thetas(2) = 2.0;
    for (int j = 0; j < p; ++j) {
      const madai::Parameter & param = m_Parent->m_Parameters[j];
      const madai::Distribution * priordist = param.m_PriorDistribution;
      m_Thetas(3+j) = scale * std::abs(priordist->GetPercentile(0.75)
                                     - priordist->GetPercentile(0.25));
    }
    break;
  case GaussianProcessEmulator::MATERN_32_FUNCTION:
    // fall through
  case GaussianProcessEmulator::MATERN_52_FUNCTION:
    m_Thetas.resize(3);
    m_Thetas(0) = amplitude;
    m_Thetas(1) = defaultNugget;
    {
      double min = std::numeric_limits< double >::max();
      for (int j = 0; j < p; ++j) {
        const madai::Parameter & param = m_Parent->m_Parameters[j];
        const madai::Distribution * priordist = param.m_PriorDistribution;
        double d = std::abs(priordist->GetPercentile(0.75)
                          - priordist->GetPercentile(0.25));
        if (d < min)
          min = d;
      }
      m_Thetas(2) = min * scale;
    }
    break;
  default:
    assert(false);
    std::cerr << "Unknown covariance function.\n";
    return false;
  }

  m_Thetas(0) = amplitude;
  m_Thetas(1) = defaultNugget;
  scale = std::abs(scale);

  if (m_CovarianceFunction ==
      GaussianProcessEmulator::POWER_EXPONENTIAL_FUNCTION)
    m_Thetas(2) = 2.0; // default power.

  for (int j = 0; j < p; ++j) {
    const madai::Parameter & param = m_Parent->m_Parameters[j];
    const madai::Distribution * priordist = param.GetPriorDistribution();
    m_Thetas(offset + j) = scale * std::abs(priordist->GetPercentile(0.75)
                                            - priordist->GetPercentile(0.25));
  }
  return true;
}

bool GaussianProcessEmulator::RetainPrincipalComponents(
  double fractionResolvingPower )
{
  if (fractionResolvingPower <= 0.0) {
    /* \todo error message to stderr */
    return false;
  }
  if (fractionResolvingPower > 1.0) {
    /* \todo error message to stderr */
    return false;
  }

  int t = m_NumberOutputs;

  double resolving_power = 1.0;
  for (int i = t-1; i >= 0; --i)
    resolving_power *= std::sqrt(1.0 + m_PCAEigenvalues(i));
  double target_resolving_power = resolving_power * fractionResolvingPower;

  resolving_power = 1.0;
  for (int i = t-1; i >= 0; --i) {
    resolving_power *= std::sqrt(1.0 + m_PCAEigenvalues(i));
    if (resolving_power >= target_resolving_power) {
      m_NumberPCAOutputs = t - i;
      break;
    }
  }

  int r = m_NumberPCAOutputs;
  assert((r > 0) && (r <= t));
  m_RetainedPCAEigenvalues = m_PCAEigenvalues.tail(r);
  m_RetainedPCAEigenvectors = m_PCAEigenvectors.rightCols(r);

  m_PCADecomposedModels.resize( m_NumberPCAOutputs );

  if ( !this->BuildZVectors() ) {
    return false;
  }

  return true;
}

bool GaussianProcessEmulator::PrincipalComponentDecompose()
{
  int t = m_NumberOutputs;
  int N = m_NumberTrainingPoints;

  // FIND PCA DECOMPOSITION OF m_OutputValues - m_OutputMeans
  m_OutputMeans = m_OutputValues.colwise().mean();
  Eigen::MatrixXd Y_minus_means
    = m_OutputValues.rowwise() - (m_OutputValues.colwise().mean());

  Eigen::MatrixXd Y_standardized(N,t);
  for (int outputIndex = 0; outputIndex < t; ++outputIndex) {
    if ( m_OutputUncertaintyScales(outputIndex) == 0.0 ) {
      std::cerr << "Output uncertainty scale is 0.0" << std::endl;
      return false;
    }
    double oneOverUncertaintyScale
      = 1.0 / m_OutputUncertaintyScales(outputIndex);
    for (int pointIndex = 0; pointIndex < N; ++pointIndex) {
      Y_standardized(pointIndex, outputIndex)
        = oneOverUncertaintyScale * Y_minus_means(pointIndex, outputIndex);
    }
  }

  Eigen::MatrixXd Ycov
    = (1.0 / N ) * Y_standardized.transpose() * Y_standardized;

  Eigen::SelfAdjointEigenSolver< Eigen::MatrixXd > eigenSolver(Ycov);

  m_PCAEigenvalues = eigenSolver.eigenvalues();
  m_PCAEigenvectors = eigenSolver.eigenvectors();

  return true;
}


/**
   Execute the model at an input point x.  Save a lot of time by not
   calculating the error. */
bool GaussianProcessEmulator::SingleModel::GetEmulatorOutputs (
    const std::vector< double > & x,
    double & mean) const {
  assert(m_RegressionOrder >= 0);
  // copy the point from vector<double> into VectorXd
  Eigen::VectorXd point = Eigen::Map<const Eigen::VectorXd>(&(x[0]),x.size());
  int N = m_Parent->m_NumberTrainingPoints;
  int p = m_Parent->m_NumberParameters;
  assert(p > 0);
  int F = 1 + (m_RegressionOrder * p);
  const Eigen::MatrixXd & X = m_Parent->m_ParameterValues;
  Eigen::VectorXd kplus(N); // kplus is C(x,D)
  for (int j = 0; j < N; ++j) {
    Eigen::VectorXd xrow = X.row(j);
    double cov = this->CovarianceCalc(xrow, point);
    if(cov < 1e-10)
      cov = 0.0;
    kplus(j) = cov;
  }
  Eigen::VectorXd h_vector(F);
  MakeHVector(point,h_vector,m_RegressionOrder);

  // m_CInverse
  //   = CMatrix.ldlt().solve(Eigen::MatrixXd::Identity(N,N));
  // m_RegressionMatrix1
  //   = (HMatrix.transpose() * m_CInverse
  //       * HMatrix).ldlt().solve(Eigen::MatrixXd::Identity(F,F));
  // m_RegressionMatrix2
  //     = (m_CInverse * HMatrix).transpose(); //[FxN]
  // m_BetaVector
  //      = m_RegressionMatrix1 * HMatrix.transpose() * m_CInverse * m_ZValues;
  // m_GammaVector = m_CInverse * (m_ZValues - (HMatrix * m_BetaVector));
  mean = h_vector.dot(m_BetaVector) + kplus.dot(m_GammaVector);
  return true;
}
/**
   Execute the model at an input point x.  Save a lot of time by not
   calculating the covaraince error. */
bool GaussianProcessEmulator::GetEmulatorOutputs (
    const std::vector< double > & x,
    std::vector< double > & y) const
{
  if (m_Status != READY) {
    std::cerr << "GetEmulatorOutputs ERROR."
      " GaussianProcessEmulator is not ready.\n";
    return false;
  }
  Eigen::VectorXd mean_pca(m_NumberPCAOutputs);

  bool errorflag = false;
#if defined( OPENMP_FOUND )
  #pragma omp parallel for
#endif // OPENMP_FOUND
  for (int i = 0; i < m_NumberPCAOutputs; ++i) {
    double d;
    if(! m_PCADecomposedModels[i].GetEmulatorOutputs(x, d)) {
      std::cerr << "error in SingleModel::GetEmulatorOutputs()\n";
      errorflag = true;
    }
    mean_pca(i) = d;
  }
  if (errorflag)
    return false;
  y.resize(m_NumberOutputs);
  Eigen::Map< Eigen::VectorXd > mean(&(y[0]),m_NumberOutputs);
  mean = m_OutputMeans +
    m_OutputUncertaintyScales.cwiseProduct(m_RetainedPCAEigenvectors * mean_pca);
  return true;
}

/**
   Execute the model at an input point x.  Save a lot of time by not
   calculating the error. */
bool GaussianProcessEmulator::SingleModel
::GetEmulatorOutputsAndCovariance (
    const std::vector< double > & x,
    double & mean,
    double & variance) const {
  assert(m_RegressionOrder >= 0);
  Eigen::Map<const Eigen::VectorXd> point(&(x[0]),x.size());
  int N = m_Parent->m_NumberTrainingPoints;
  int p = m_Parent->m_NumberParameters;
  assert(p > 0);
  int F = 1 + (m_RegressionOrder * p);
  const Eigen::MatrixXd & X = m_Parent->m_ParameterValues;
  Eigen::VectorXd kplus(N);
  // kplus is C(x,D)
  for (int j = 0; j < N; ++j) {
    Eigen::VectorXd xrow = X.row(j);
    double cov = this->CovarianceCalc(xrow, point);
    if(cov < 1e-10)
      cov = 0.0;
    kplus(j) = cov;
  }
  Eigen::VectorXd h_vector(F);
  h_vector(0) = 1.0;
  if (m_RegressionOrder > 0)
    h_vector.segment(1,p) = point;
  for (int i = 2; i < m_RegressionOrder; ++i) {
    h_vector.segment(1+(i*p),p)
      = h_vector.segment(1+((i-1)*p),p).cwiseProduct(point);
  }
  // m_CInverse = CMatrix.ldlt().solve(Eigen::MatrixXd::Identity(N,N));
  // m_RegressionMatrix
  //    = (HMatrix.transpose() * m_CInverse
  //         * HMatrix).ldlt().solve(Eigen::MatrixXd::Identity(F,F));
  // m_BetaVector
  //     = m_RegressionMatrix * HMatrix.transpose() * m_CInverse * m_ZValues;
  // m_GammaVector = m_CInverse * (m_ZValues - (HMatrix * m_BetaVector));
  mean = h_vector.dot(m_BetaVector) + kplus.dot(m_GammaVector);

  // Eigen::VectorXd  f = h_vector - kplus.dot(m_CInverse * h_vector);
  // variance = this->CovarianceCalc(point, point)
  //   - kplus.dot(m_CInverse * kplus) + f.dot(m_RegressionMatrix * f);

  // m_RegressionMatrix2
  //     = (m_CInverse * HMatrix).transpose(); //[FxN]

  Eigen::VectorXd  f = h_vector - (m_RegressionMatrix2 * kplus);

  variance = this->CovarianceCalc(point, point)
    - kplus.dot(m_CInverse * kplus) + f.dot(m_RegressionMatrix1 * f);
  return true;
}

/**
         Execute the model at an input point x.
         The covariance returned will be a flattened matrix */
bool GaussianProcessEmulator::GetEmulatorOutputsAndCovariance (
    const std::vector< double > & x,
    std::vector< double > & y,
    std::vector< double > & ycov) const {
  if (m_Status != READY)
    return false;

  Eigen::Map<const Eigen::VectorXd> point(&(x[0]),x.size());
  int t = m_NumberOutputs;
  Eigen::VectorXd mean_pca(m_NumberPCAOutputs);
  Eigen::VectorXd var_pca(m_NumberPCAOutputs);
  for (int i = 0; i < m_NumberPCAOutputs; ++i) {
    double smean, variance;
    if (! m_PCADecomposedModels[i].GetEmulatorOutputsAndCovariance(
            x, smean, variance))
      return false;
    mean_pca(i) = smean;
    var_pca(i) = variance;
  } // end-for(i < (m_NumberPCAOutputs))
  y.resize(t);
  ycov.resize(t * t);
  Eigen::Map< Eigen::VectorXd > mean(&(y[0]), t);
  Eigen::Map< Eigen::MatrixXd > covariance(&(ycov[0]), t, t);
  mean = m_OutputMeans +
    m_OutputUncertaintyScales.cwiseProduct(m_RetainedPCAEigenvectors * mean_pca);

  Eigen::MatrixXd uncertaintyScales
    = m_OutputUncertaintyScales * m_OutputUncertaintyScales.transpose();
  covariance
    = uncertaintyScales.cwiseProduct(
        m_RetainedPCAEigenvectors * var_pca.asDiagonal() *
        m_RetainedPCAEigenvectors.transpose());

  return true;
}


/**
   \returns m_Status */
GaussianProcessEmulator::StatusType
GaussianProcessEmulator::GetStatus() const {
  return m_Status;
}

} // namespace madai
