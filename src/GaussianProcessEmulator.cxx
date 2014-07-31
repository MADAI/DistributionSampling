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

#include "Configuration.h"
#include "GaussianProcessEmulator.h"
#include "GaussianDistribution.h"
#include "UniformDistribution.h"
#include "Paths.h"

#include "madaisys/Directory.hxx"

namespace madai {

// Anonymous namespace to hide these functions
namespace {

/**
 * Get the number of regression functions.
 */
inline int NumberRegressionFunctions(
    int regressionOrder,
    int numberParameters) {
  return 1 + (regressionOrder * numberParameters);
}

/**
 * Get the offset into the theta vector where the parameter scales start.
 */
inline int ThetaOffset(
    GaussianProcessEmulator::CovarianceFunctionType cf) {
  switch(cf) {
  case GaussianProcessEmulator::SQUARE_EXPONENTIAL_FUNCTION:
    return 2;
  case GaussianProcessEmulator::POWER_EXPONENTIAL_FUNCTION:
    return 3;
  case GaussianProcessEmulator::MATERN_32_FUNCTION:
    return 2;
  case GaussianProcessEmulator::MATERN_52_FUNCTION:
    return 2;
  case GaussianProcessEmulator::UNKNOWN_FUNCTION:
    //fall through
  default:
    return -1;
  }
}
inline int NumberThetas(
    GaussianProcessEmulator::CovarianceFunctionType cf,
    int numberParameters) {
  switch(cf) {
  case GaussianProcessEmulator::UNKNOWN_FUNCTION:
    return -1;
  default:
    return numberParameters + ThetaOffset(cf);
  }
}


/**
 * Compute the matrix H.
 */
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

/**
 * Compute the vector H.
 */
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

/**
 * Compute the gradient of the H vector. */
template < typename TDerived >
inline void GetGradientOfHVector(
  const Eigen::MatrixBase< TDerived > & point,
  Eigen::MatrixXd & GradMatrix,
  int regressionOrder)
{
  int p = point.size();
  int numberRegressionFunctions = 1 + (regressionOrder * p);
  GradMatrix.resize(p, numberRegressionFunctions);
  GradMatrix = Eigen::MatrixXd::Zero(p, numberRegressionFunctions);
  if ( regressionOrder > 0 ) {
    for ( int i = 0; i < p; i++ )
      GradMatrix(i,i+1) = 1.0;
    for ( int i = 1; i < regressionOrder; ++i ) {
      for ( int j = 0; j < p; j++ ) {
        GradMatrix(j,1+i*p+j) = double(i + 1) * std::pow(point(i), double(i));
      }
    }
  }
}

/**
   Score the trainingModel based on points in the originalModel; */
static double Score(
    const GaussianProcessEmulator::SingleModel & originalModel,
    const GaussianProcessEmulator::SingleModel & trainingModel)
{
  int N = originalModel.m_Parent->m_NumberTrainingPoints;
  int p = originalModel.m_Parent->m_NumberParameters;
  const Eigen::MatrixXd & orig_Params =
    originalModel.m_Parent->m_TrainingParameterValues;
  const Eigen::VectorXd & orig_ZValues = originalModel.m_ZValues;
  double score = 0.0; //low scores are better
  std::vector< double > x(p);
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < p; ++j) {
      x[j] = orig_Params(i,j);
    }
    double zval;
    if (! trainingModel.GetEmulatorOutputs(x, zval)) {
      std::cerr << "ERROR in " __FILE__ ":" << __LINE__ << "\n";
      return std::numeric_limits<double>::signaling_NaN();
    }
    score += std::pow(zval - orig_ZValues(i), 2);
  }
  return score / N;
}

/**
   Use K-fold cross-validation to test hyper-parameters. */
static double Score(
    const GaussianProcessEmulator::SingleModel & originalModel)
{
  static const int PARTITIONS = 8;

  int N = originalModel.m_Parent->m_NumberTrainingPoints;
  int p = originalModel.m_Parent->m_NumberParameters;
  const Eigen::MatrixXd & trainingParameterValues =
    originalModel.m_Parent->m_TrainingParameterValues;
  int numberToLeaveOut = N / PARTITIONS;
  int numberToKeep = N - numberToLeaveOut;
  // assume Training Points are not sorted in any particular order
  GaussianProcessEmulator dummy;
  dummy.m_NumberParameters = p;
  dummy.m_Parameters = originalModel.m_Parent->m_Parameters;
  dummy.m_NumberTrainingPoints = numberToKeep;
  dummy.m_TrainingParameterValues.resize(numberToKeep, p);
  GaussianProcessEmulator::SingleModel testModel;
  testModel.m_Parent = &dummy;
  testModel.m_CovarianceFunction = originalModel.m_CovarianceFunction;
  testModel.m_RegressionOrder = originalModel.m_RegressionOrder;
  testModel.m_Thetas.resize(originalModel.m_Thetas.size());
  testModel.m_Thetas = originalModel.m_Thetas;
  testModel.m_ZValues.resize(numberToKeep);
  double score = 0.0;
  for (int k = 0; k < PARTITIONS; ++k) {
    // K-fold cross-validation
    int firstIndexToSkip = k * numberToLeaveOut;
    for (int i = 0; i < numberToKeep; ++i) {
      int sourceIndex = (i < firstIndexToSkip) ? i : (i + numberToLeaveOut);
      for (int j = 0; j < p; ++j) {
        dummy.m_TrainingParameterValues(i,j) =
          trainingParameterValues(sourceIndex,j);
      }
      testModel.m_ZValues(i) = originalModel.m_ZValues(sourceIndex);
    }
    if ( !testModel.MakeCache() ) {
      std::cerr << "Error when making cache in test model.\n";
      std::cerr << "ERROR in " __FILE__ ":" << __LINE__ <<  "\n";
      return std::numeric_limits<double>::signaling_NaN();
    }
    score += Score(originalModel, testModel);
  }
  return score / PARTITIONS;
}


/**
   Simplest way to set length-scale hyperparameters.  scale is
   multiplied by the width of the middle-two quartiles of the prior
   distribution.  Used by both train() and basictrain().  */
static void setThetasByScale(
    GaussianProcessEmulator::SingleModel & model,
    double scale)
{
  int p = model.m_Parent->m_NumberParameters;
  int offset = ThetaOffset(model.m_CovarianceFunction);
  model.m_Thetas.resize(offset + p);
  for (int j = 0; j < p; ++j) {
    const madai::Distribution * priordist =
      model.m_Parent->m_Parameters[j].m_PriorDistribution;
    assert(priordist);
    model.m_Thetas(offset + j) = scale * std::abs(
        priordist->GetPercentile(0.75) - priordist->GetPercentile(0.25));
  }

}


} // anonymous namespace

double GaussianProcessEmulator::SingleModel::CovarianceCalc(
    const Eigen::VectorXd & v1, const Eigen::VectorXd & v2) const
{
  static const double EPSILON = 1e-10;
  int p = m_Parent->m_NumberParameters;
  int offset = ThetaOffset(m_CovarianceFunction);
  assert(offset != -1);
  assert(m_Thetas.size() == (p + offset));
  if ((offset == -1) || (m_Thetas.size() != (p + offset))) {
    // Only if assertions are disabled
    return 0.0; // we should throw an exception.
  }
  assert(offset >= 2);
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


bool GaussianProcessEmulator::SingleModel::GetGradientOfCovarianceCalc(
    const Eigen::VectorXd & v1, const Eigen::VectorXd & v2,
    Eigen::VectorXd & gradient) const
{
  int p = m_Parent->m_NumberParameters;
  int offset = ThetaOffset(m_CovarianceFunction);
  assert(offset != -1);
  assert(m_Thetas.size() == (p + offset));
  if ((offset == -1) || (m_Thetas.size() != (p + offset))) {
    return false;
  }
  const double & amplitude = m_Thetas(0);
  //const double & nugget = m_Thetas(1);

  double distanceSquared = 0.0;
  for (int i = 0; i < p; i++) {
    double d = v1(i) - v2(i);
    double l = m_Thetas(i + offset);
    distanceSquared += std::pow( (d / l), 2);
  }

  gradient.resize(p);
  switch(m_CovarianceFunction) {
  case POWER_EXPONENTIAL_FUNCTION:
    {
      const double & power = m_Thetas(2);
      assert((power > 0.0) && (power <= 2.0));
      double covariance = this->CovarianceCalc( v1, v2 );
      for(int i = 0; i < p; i++ ) {
        double sign;
        if(v1(i) < v2(i)) {
          sign = -1;
        } else {
          sign = 1;
        }
        gradient(i) = -sign*amplitude*power*covariance
        *std::pow(std::abs(v1(i) - v2(i)),(power-1))
        /(2.0*std::pow(m_Thetas(i + offset),power));
      }
      return true;
    }
  case SQUARE_EXPONENTIAL_FUNCTION:
    {
      double covariance = this->CovarianceCalc( v1, v2 );
      for(int i = 0; i < p; i++ ) {
        gradient(i) = -amplitude*(v1(i) - v2(i))*covariance
        /std::pow( m_Thetas(i + offset), 2);
      }
      return true;
    }
  case MATERN_32_FUNCTION:
    {
      static const double ROOT3 = 1.7320508075688772;
      double distance = std::sqrt(distanceSquared);
      for(int i = 0; i < p; i++ ) {
        gradient(i) = -3.0*amplitude*(v1(i) - v2(i))
        *std::exp(-ROOT3*distance)
        /std::pow(m_Thetas(i + offset), 2);
      }
      return true;
    }
  case MATERN_52_FUNCTION:
    {
      static const double ROOT5 = 2.23606797749979;
      double distance = std::sqrt(distanceSquared);
      for(int i = 0; i < p; i++ ) {
        gradient(i) = -5.0*amplitude*(v1(i) - v2(i))
        *(1.0-ROOT5*distance)*std::exp(-ROOT5*distance)
        /(3.0*std::pow(m_Thetas(i + 2),2));
      }
    }
  default:
    assert(false);
    return false;
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
  if (m_TrainingParameterValues.rows() != m_NumberTrainingPoints) {
    return m_Status;
  }
  if (m_TrainingOutputValues.rows() != m_NumberTrainingPoints) {
    return m_Status;
  }
  if (m_TrainingParameterValues.cols() != m_NumberParameters) {
    return m_Status;
  }
  if (m_TrainingOutputValues.cols() != m_NumberOutputs) {
    return m_Status;
  }
  if(m_TrainingOutputVarianceMeans.size() != m_NumberOutputs) {
    m_TrainingOutputVarianceMeans
      = Eigen::VectorXd::Constant(m_NumberOutputs,0.0);
  }
  if(m_ObservedValues.size() != m_NumberOutputs) {
    m_ObservedValues
      = Eigen::VectorXd::Constant(m_NumberOutputs,0.0);
  }
  m_Status = UNTRAINED;
  if(m_NumberPCAOutputs < 1)
    return m_Status;
  if(m_TrainingOutputMeans.size() != m_NumberOutputs)
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


GaussianProcessEmulator::StatusType
GaussianProcessEmulator::GetStatus() const {
  return m_Status;
}


std::string
GaussianProcessEmulator::GetStatusAsString() const
{
  switch ( m_Status ) {
  case GaussianProcessEmulator::READY:
    return std::string( "READY" );
    break;

  case GaussianProcessEmulator::UNCACHED:
    return std::string( "UNCACHED" );
    break;

  case GaussianProcessEmulator::UNTRAINED:
    return std::string( "UNTRAINED" );
    break;

  case GaussianProcessEmulator::UNINITIALIZED:
    return std::string( "UNINITIALIZED" );
    break;

  case GaussianProcessEmulator::ERROR:
  default: return std::string( "ERROR" );
  }

  return std::string();
}


bool GaussianProcessEmulator::GetUncertaintyScales(
    std::vector< double > & x) const
{
  if(m_UncertaintyScales.size() != m_NumberOutputs) {
    GaussianProcessEmulator * nonConstGPE = const_cast< GaussianProcessEmulator * >( this );
    if ( !nonConstGPE->BuildUncertaintyScales() ) {
      x.clear();
      return false;
    }
  }

  x.resize(m_NumberOutputs);
  for (int i = 0; i < m_NumberOutputs; ++i) {
    x[i] = m_UncertaintyScales(i);
  }

  return true;
}

bool GaussianProcessEmulator::GetUncertaintyScalesAsCovariance(
    std::vector< double > & x) const
{
  x.clear();
  int t = m_NumberOutputs;
  if( m_UncertaintyScales.size() != t ) {
    GaussianProcessEmulator * nonConstGPE = const_cast< GaussianProcessEmulator * >( this );
    if ( !nonConstGPE->BuildUncertaintyScales() ) {
      return false;
    }
  }

  x.resize(t*t);
  Eigen::Map< Eigen::MatrixXd > Cov(&(x[0]),t,t);
  Cov = m_UncertaintyScales.asDiagonal();
  return true;
}

bool GaussianProcessEmulator::GetObservedValues(
    std::vector< double > & x)
{
  if(m_ObservedValues.size() != m_NumberOutputs) {
    x.assign(m_NumberOutputs,0.0);
  } else {
    x.resize(m_NumberOutputs);
    for (int i = 0; i < m_NumberOutputs; ++i)
      x[i] = m_ObservedValues(i);
  }
  return true;
}

GaussianProcessEmulator::SingleModel::SingleModel() :
  m_Parent(NULL),
  m_CovarianceFunction(UNKNOWN_FUNCTION),
  m_RegressionOrder(-1)
{ }

bool GaussianProcessEmulator::BuildUncertaintyScales()
{
  if ( m_TrainingOutputVarianceMeans.size() != m_NumberOutputs ) {
    std::cerr << "Error in "
              << "GaussianProcessEmulator::BuildUncertaintyScales():\n";
    std::cerr << "  m_TrainingOutputVarianceMeans.size() != m_NumberOutputs\n";
    return false;
  }

  if ( m_ObservedVariances.size() != m_NumberOutputs ) {
    std::cerr << "Error in "
              << "GaussianProcessEmulator::BuildUncertaintyScales():\n";
    std::cerr << "  m_ObservedVariances.size() != m_NumberOutputs\n";
    return false;
  }

  // Compute uncertainty scales.
  m_UncertaintyScales = Eigen::VectorXd::Constant( m_NumberOutputs, 0.0 );
  for ( int i = 0; i < m_TrainingOutputVarianceMeans.size(); ++i ) {
    if ( m_UseModelUncertainty ) {
      m_UncertaintyScales( i ) = std::sqrt(
        std::pow( m_TrainingOutputVarianceMeans( i ), 2 ) +
        std::pow( m_ObservedVariances(i), 2 ) );
    }
    else {
      m_UncertaintyScales( i ) = m_ObservedVariances(i);
    }
  }

  return true;
}

bool GaussianProcessEmulator::BuildZVectors() {
  if (m_PCADecomposedModels.size() != static_cast< size_t >( m_NumberPCAOutputs ) ) {
    std::cerr << "Error [m_PCADecomposedModels.size() == "
              << m_PCADecomposedModels.size() << " != m_NumberPCAOutputs == "
              << m_NumberPCAOutputs << "]\n";
    return false;
  }

  std::vector< double > uncertaintyScales;
  if ( !this->GetUncertaintyScales( uncertaintyScales ) ) {
    return false;
  }

  Eigen::MatrixXd Y_standardized(m_NumberTrainingPoints, m_NumberOutputs);
  for (int i = 0; i < m_NumberOutputs; ++i) {
    double scale = 1.0 / uncertaintyScales[i];
    for (int j = 0; j < m_NumberTrainingPoints; ++j) {
      Y_standardized(j, i)
        = scale * (m_TrainingOutputValues(j,i) - m_TrainingOutputMeans(i));
    }
  }
  Eigen::MatrixXd zMatrix = Y_standardized * m_RetainedPCAEigenvectors;
  for (int i = 0; i < m_NumberPCAOutputs; ++i) {
    GaussianProcessEmulator::SingleModel & m = m_PCADecomposedModels[i];
    m.m_ZValues = zMatrix.col(i);
  }
  return true;
}

bool GaussianProcessEmulator::MakeCache() {
  if ((m_Status != READY) && (m_Status != UNCACHED)) {
    std::cerr << "ERROR in " __FILE__ ":" << __LINE__ << "\n";
    return false;
  }
  assert(m_NumberPCAOutputs == static_cast<int>(m_PCADecomposedModels.size()));
  bool errorflag = false;
#if defined( OPENMP_FOUND )
  #pragma omp parallel for
#endif // OPENMP_FOUND
  for (int i = 0; i < m_NumberPCAOutputs; ++i) {
    if (! m_PCADecomposedModels[i].MakeCache()) {
      std::cerr << "ERROR in " __FILE__ ":" << __LINE__ << " (" << i << ")\n";
      errorflag = true;
    }
  }
  if (errorflag) {
    return false;
  }
  m_Status = READY;
  return true;
}

bool GaussianProcessEmulator::SingleModel::MakeCache() {
  int N = m_Parent->m_NumberTrainingPoints;
  int p = m_Parent->m_NumberParameters;
  int F = NumberRegressionFunctions(m_RegressionOrder, p);
  const Eigen::MatrixXd & X = m_Parent->m_TrainingParameterValues;

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

GaussianProcessEmulator::GaussianProcessEmulator(bool useModelUncertainty) :
  m_UseModelUncertainty(useModelUncertainty),
  m_Status(UNINITIALIZED),
  m_NumberParameters(0),
  m_NumberOutputs(0),
  m_NumberTrainingPoints(0),
  m_NumberPCAOutputs(0)
{ }


bool GaussianProcessEmulator::SingleModel::Train(
    GaussianProcessEmulator::CovarianceFunctionType covarianceFunction,
    int regressionOrder)
{
  int N = m_Parent->m_NumberTrainingPoints;
  if ((regressionOrder < 0) || (regressionOrder > 3)) {
    std::cerr << "regressionOrder out of range (" << regressionOrder << ")\n";
    return false;
  }
  m_CovarianceFunction = covarianceFunction;
  m_RegressionOrder = regressionOrder;
  int offset = ThetaOffset(m_CovarianceFunction);
  m_Thetas.resize(offset + m_Parent->m_NumberParameters);

  // FIXME - are we sensitive to these hyperparameters?
  m_Thetas(0) = 1.0; // default value    // amplitude
  m_Thetas(1) = 1.0e-5; // default value // nugget
  if (offset == 3)
    m_Thetas(2) = 2.0; // default value  // power

  double minimumScale = 4.0 / static_cast<double>(N);
  double maximumScale = 1.0;
  static const int NUMBER_OF_TRIES = 20; // assert (NUMBER_OF_TRIES > 0);

  double factor
    = std::pow(maximumScale / minimumScale, 1.0 / (NUMBER_OF_TRIES - 1));
  double scale = minimumScale;
  double bestScale = std::numeric_limits<double>::signaling_NaN();
  double lowestScore = std::numeric_limits<double>::infinity();
  for (int i = 0; i < NUMBER_OF_TRIES; ++i) {
    setThetasByScale(*this, scale);
    /* This as a hack.  We assume that the best scale works equally
       well for each parameter.  There is no reason to believe this. */
    double score = Score(*this);
    if (score < lowestScore) {
      lowestScore = score;
      bestScale = scale;
    }
    scale *= factor;
  }
  setThetasByScale(*this, bestScale);
  return true;
}


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
  bool errorflag = false;
  #if defined( OPENMP_FOUND )
  #pragma omp parallel for
  #endif // OPENMP_FOUND
  for (int i = 0; i < t; ++i) {
    if (! m_PCADecomposedModels[i].Train(covarianceFunction,regressionOrder)) {
      std::cerr << "errror in GaussianProcessEmulator::SingleModel::Train()\n";
      errorflag = true;
    }
  }
  if (errorflag)
    return false;
  m_Status = UNCACHED;
  if (! this->MakeCache()) {
    std::cerr << "Error in this->MakeCache()\n  " __FILE__ ":"
              << __LINE__ << '\n';
    return false;
  }
  return true;
}

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

bool GaussianProcessEmulator::SingleModel::BasicTraining(
    CovarianceFunctionType covarianceFunction,
    int regressionOrder,
    double defaultNugget,
    double amplitude,
    double scale) {
  m_CovarianceFunction = covarianceFunction;
  m_RegressionOrder = regressionOrder;
  scale = std::abs(scale);
  int p = m_Parent->m_NumberParameters;
  int offset = ThetaOffset(m_CovarianceFunction);
  assert(offset != -1); // error condition
  if (offset == -1) {
    return false;
  }
  m_Thetas.resize(offset + p);

  if (m_CovarianceFunction ==
      GaussianProcessEmulator::POWER_EXPONENTIAL_FUNCTION) {
    assert(offset == 3);
    m_Thetas(2) = 2.0; // default value
  } else {
    assert(offset == 2);
  }
  m_Thetas(0) = amplitude;
  m_Thetas(1) = defaultNugget;

  setThetasByScale(*this, scale);
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

  std::vector< double > uncertaintyScales;
  if ( !this->GetUncertaintyScales( uncertaintyScales ) ) {
    return false;
  }

  // FIND PCA DECOMPOSITION OF m_TrainingOutputValues - m_TrainingOutputMeans
  m_TrainingOutputMeans = m_TrainingOutputValues.colwise().mean();
  Eigen::MatrixXd Y_minus_means
    = m_TrainingOutputValues.rowwise() - (m_TrainingOutputValues.colwise().mean());

  Eigen::MatrixXd Y_standardized(N,t);
  for (int outputIndex = 0; outputIndex < t; ++outputIndex) {
    if ( uncertaintyScales[outputIndex] == 0.0 ) {
      std::cerr << "Output uncertainty scale is 0.0, which is invalid\n";
      return false;
    }
    double oneOverUncertaintyScale
      = 1.0 / uncertaintyScales[outputIndex];
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
  const Eigen::MatrixXd & X = m_Parent->m_TrainingParameterValues;
  Eigen::VectorXd kplus(N); // kplus is C(x,D)
  assert((X.rows() == N) && (X.cols() == p));
  for (int j = 0; j < N; ++j) {
    Eigen::VectorXd xrow(p);
    for (int k = 0; k < p; ++k) {
      xrow(k) = X(j,k);
    }
    // Eigen::VectorXd xrow = X.row(j); /* is buggy? */
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

bool GaussianProcessEmulator::SingleModel::GetGradientOfEmulatorOutputs(
    const std::vector< double > & x,
    std::vector< double > & gradient ) const
{
  assert(m_RegressionOrder >= 0);
  gradient.clear();
  // copy the point from vector<double> into VectorXd
  Eigen::VectorXd point = Eigen::Map<const Eigen::VectorXd>(&(x[0]),x.size());
  int N = m_Parent->m_NumberTrainingPoints;
  int p = m_Parent->m_NumberParameters;
  gradient.resize(p);
  Eigen::Map< Eigen::VectorXd > ModelGradient(&(gradient[0]), p);
  assert(p>0);
  const Eigen::MatrixXd & X = m_Parent->m_TrainingParameterValues;
  // Get Gradient of the covariance
  Eigen::MatrixXd cov_grad(p, N);
  for ( int i = 0; i < N; i++ ) {
    Eigen::VectorXd Grad;
    this->GetGradientOfCovarianceCalc( point, X.row(i), Grad  );
    cov_grad.col(i) = Grad;
  }
  ModelGradient = cov_grad*m_GammaVector;
  // Get gradients of h_vector
  Eigen::MatrixXd h_v_Grad;
  GetGradientOfHVector(point, h_v_Grad, m_RegressionOrder);
  ModelGradient += h_v_Grad*m_BetaVector;
  return true;
}

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
  mean = m_TrainingOutputMeans +
    m_UncertaintyScales.cwiseProduct(m_RetainedPCAEigenvectors * mean_pca);
  return true;
}

bool GaussianProcessEmulator::GetGradientOfEmulatorOutputs(
    const std::vector< double > & x,
    std::vector< double > & gradients ) const
{
  if ( m_Status != READY ) {
    std::cerr << "GetGradientsOfEmulatorOutputs ERROR."
    " GaussianProcessEmulator is not ready.\n";
    return false;
  }
  gradients.clear();

  int p = m_NumberParameters;
  int t = m_NumberPCAOutputs;
  int o = m_NumberOutputs;
  std::vector< double > grad;
  Eigen::MatrixXd mean_pca_gradients( t, p );
  for ( int i = 0; i < t; i++ ) {
    if ( !m_PCADecomposedModels[i].GetGradientOfEmulatorOutputs( x, grad ) )
      return false;
    mean_pca_gradients.row(i) = Eigen::Map< Eigen::RowVectorXd >(&(grad[0]),p);
  }
  gradients.resize(o*p);
  Eigen::Map< Eigen::VectorXd > OutputGradients(&(gradients[0]),(o*p));
  for ( int i = 0; i < p; i++ ) {
    OutputGradients.segment((i*o), o) = m_UncertaintyScales.cwiseProduct(
        m_RetainedPCAEigenvectors * mean_pca_gradients.col(i) );
  }
  return true;
}

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
  const Eigen::MatrixXd & X = m_Parent->m_TrainingParameterValues;
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

bool GaussianProcessEmulator::SingleModel
::GetGradientOfCovariance(
    const std::vector< double > & x,
    std::vector< double > & gradient) const
{
  assert(m_RegressionOrder >= 0);
  gradient.clear();
  // copy the point from vector<double> into VectorXd
  Eigen::VectorXd point = Eigen::Map<const Eigen::VectorXd>(&(x[0]),x.size());
  int N = m_Parent->m_NumberTrainingPoints;
  int p = m_Parent->m_NumberParameters;
  gradient.resize(p);
  Eigen::Map< Eigen::VectorXd > ModelGradient(&(gradient[0]), p);
  assert(p>0);
  int F = 1 + (m_RegressionOrder * p);
  const Eigen::MatrixXd & X = m_Parent->m_TrainingParameterValues;
  // Get Gradient of the covariance
  Eigen::MatrixXd cov_grad(p, N);
  for ( int i = 0; i < N; i++ ) {
    Eigen::VectorXd Grad;
    this->GetGradientOfCovarianceCalc( point, X.row(i), Grad  );
    cov_grad.col(i) = Grad;
  }
  Eigen::VectorXd kplus(N);
  for (int j = 0; j < N; ++j) {
    double cov = this->CovarianceCalc( X.row(j), point);
    if(cov < 1e-10)
      cov = 0.0;
    kplus(j) = cov;
  }
  // Get gradients of h_vector
  Eigen::MatrixXd h_v_Grad; // p,(1+ro*p)
  GetGradientOfHVector(point, h_v_Grad, m_RegressionOrder);
  Eigen::VectorXd h_vector(F);
  MakeHVector(point,h_vector,m_RegressionOrder);
  // Calculate gradient of the variance
  ModelGradient = -cov_grad*m_CInverse*kplus
                  -(kplus.transpose()*m_CInverse*cov_grad.transpose()).transpose();
  Eigen::MatrixXd tm = h_v_Grad.transpose()-m_RegressionMatrix2*cov_grad.transpose();
  Eigen::VectorXd tv = h_vector-m_RegressionMatrix2*kplus;
  ModelGradient += tm.transpose()*m_RegressionMatrix1*tv
                + (tv.transpose()*m_RegressionMatrix1*tm).transpose();
  return true;
}

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
  mean = m_TrainingOutputMeans +
    m_UncertaintyScales.cwiseProduct(m_RetainedPCAEigenvectors * mean_pca);

  Eigen::MatrixXd uncertaintyScales
    = m_UncertaintyScales * m_UncertaintyScales.transpose();
  covariance
    = uncertaintyScales.cwiseProduct(
        m_RetainedPCAEigenvectors * var_pca.asDiagonal() *
        m_RetainedPCAEigenvectors.transpose());

  return true;
}

bool GaussianProcessEmulator::GetGradientsOfCovariances(
    const std::vector< double > & x,
    std::vector< Eigen::MatrixXd > & gradients ) const
{
  if (m_Status != READY)
    return false;

  Eigen::Map<const Eigen::VectorXd> point(&(x[0]),x.size());
  int p = m_NumberParameters;

  Eigen::MatrixXd var_grads(m_NumberPCAOutputs, p);
  for ( int i = 0; i < m_NumberPCAOutputs; i++ ) {
    std::vector< double > tg;
    if (! m_PCADecomposedModels[i].GetGradientOfCovariance( x, tg ) )
      return false;
    var_grads.row(i) = Eigen::Map< Eigen::VectorXd >(&(tg[0]),p);
  }

  Eigen::MatrixXd uncertaintyScales
  = m_UncertaintyScales * m_UncertaintyScales.transpose();

  for ( int i = 0; i < p; i++ ) {
    gradients.push_back( uncertaintyScales.cwiseProduct( m_RetainedPCAEigenvectors
            * var_grads.col(i).asDiagonal() * m_RetainedPCAEigenvectors.transpose() ) );
  }
  return true;
}

} // namespace madai
