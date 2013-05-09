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

#include "GaussianProcessEmulatorSingleFileWriter.h"

#include "GaussianDistribution.h"
#include "GaussianProcessEmulator.h"
#include "UniformDistribution.h"

#include <ostream>
#include <string>
#include <vector>


namespace madai {

GaussianProcessEmulatorSingleFileWriter
::GaussianProcessEmulatorSingleFileWriter()
{
}


GaussianProcessEmulatorSingleFileWriter
::~GaussianProcessEmulatorSingleFileWriter()
{
}


namespace {

std::ostream & serializeComments(
    const std::vector< std::string > & comments,
    std::ostream & o) {
  for (unsigned int i = 0; i < comments.size(); ++i) {
    o << comments[i] << '\n';
  }
  return o;
}

/**
   Print a Matrix to output stream, preceded by its dimensions.  Use
   row-major order. */
template < typename TDerived >
static inline std::ostream & PrintMatrix(
    const Eigen::MatrixBase< TDerived > & m,
    std::ostream & o)
{
  o << m.rows() << ' ' << m.cols() << '\n';
  if (m.cols() > 0) {
    for (int i = 0; i < m.rows(); ++i) {
      for (int j = 0; j < (m.cols() - 1); ++j)
        o << m(i, j) << '\t';
      o << m(i, m.cols() - 1) << '\n';
    }
  }
  return o;
}

/**
   Print a Vector to output stream, preceded by its size. */
template < typename TDerived >
static inline std::ostream & PrintVector(
    const Eigen::MatrixBase< TDerived > & v,
    std::ostream & o)
{
  o << v.size() << '\n';
  for (int i = 0; i < v.size(); ++i) {
    o << v(i) << '\n';
  }
  return o;
}


/**
 * A covariance function can be represented as a string. */
const char * GetCovarianceFunctionString(
  GaussianProcessEmulator::CovarianceFunctionType cov)
{
  switch (cov) {
  case GaussianProcessEmulator::POWER_EXPONENTIAL_FUNCTION:  return "POWER_EXPONENTIAL_FUNCTION";
  case GaussianProcessEmulator::SQUARE_EXPONENTIAL_FUNCTION: return "SQUARE_EXPONENTIAL_FUNCTION";
  case GaussianProcessEmulator::MATERN_32_FUNCTION:  return "MATERN_32_FUNCTION";
  case GaussianProcessEmulator::MATERN_52_FUNCTION:  return "MATERN_52_FUNCTION";
  default:
    assert(false);
    return "UNKNOWN";
  }
}

std::ostream & serializeParameters(
    const std::vector<madai::Parameter> & parameters,
    std::ostream & o) {
  int numberParameters = parameters.size();
  o << numberParameters << '\n';
  for(int i = 0; i < numberParameters; ++i) {
    o << parameters[i].m_Name << '\t';
    const Distribution * priorDist = parameters[i].GetPriorDistribution();
    const UniformDistribution * uniformPriorDist
      = dynamic_cast<const UniformDistribution *>(priorDist);
    const GaussianDistribution * gaussianPriorDist
      = dynamic_cast<const GaussianDistribution *>(priorDist);
    if (uniformPriorDist != NULL) {
      o << "UNIFORM" << '\t'
        << uniformPriorDist->GetMinimum() << '\t'
        << uniformPriorDist->GetMaximum() << '\n';
    } else if (gaussianPriorDist != NULL) {
      o << "GAUSSIAN" << '\t'
        << gaussianPriorDist->GetMean() << '\t'
        << gaussianPriorDist->GetStandardDeviation() << '\n';
    } else {
      assert(false);
      o << "UNKNOWN_PRIOR_TYPE\t0\t1\n";
    }
  }
  return o;
}


std::ostream & serializeStringVector(
    const std::vector< std::string > & strings,
    std::ostream & o) {
  o << strings.size() << '\n';
  for(unsigned int i = 0; i < strings.size(); ++i) {
    o << strings[i] << '\n';
  }
  return o;
}


std::ostream & serializeSubmodels(
    const GaussianProcessEmulator::SingleModel & m,
    int modelIndex,
    std::ostream & o) {
  o << "MODEL " << modelIndex << '\n';
  o << "COVARIANCE_FUNCTION\t"
    << GetCovarianceFunctionString(m.m_CovarianceFunction) << '\n';
  o << "REGRESSION_ORDER\t" << m.m_RegressionOrder << '\n';
  o << "THETAS\n";
  PrintVector(m.m_Thetas, o);
  o << "END_OF_MODEL\n";
  return o;
}

std::ostream & serializeEmulatorData(
    const GaussianProcessEmulator & gpme,
    std::ostream & o ) {
  o << "SUBMODELS\t"
    << gpme.m_NumberPCAOutputs << "\n";
  for ( int i = 0; i < gpme.m_NumberPCAOutputs; i++ ) {
    serializeSubmodels( gpme.m_PCADecomposedModels[i], i, o );
  }
  o << "END_OF_FILE\n";
  return o;
}

std::ostream & serializeGaussianProcessEmulator(
    const GaussianProcessEmulator & gpme,
    std::ostream & o) {
  o << "SUBMODELS\t"
    << gpme.m_NumberPCAOutputs << "\n";
  for (int i = 0; i < gpme.m_NumberPCAOutputs; ++i) {
    serializeSubmodels(gpme.m_PCADecomposedModels[i],i,o);
  }
  o << "END_OF_FILE\n";
  return o;
}

std::ostream & serializePCADecomposition(
    const GaussianProcessEmulator & gpme,
    std::ostream & o ) {
  serializeComments(gpme.m_Comments,o);
  o << "OUTPUT_MEANS\n";
  PrintVector(gpme.m_TrainingOutputMeans, o);
  o << "OUTPUT_UNCERTAINTY_SCALES\n";
  PrintVector(gpme.m_UncertaintyScales, o);
  o << "OUTPUT_PCA_EIGENVALUES\n";
  PrintVector(gpme.m_PCAEigenvalues, o);
  o << "OUTPUT_PCA_EIGENVECTORS\n";
  PrintMatrix(gpme.m_PCAEigenvectors, o);
  o << "END_OF_FILE\n";
  return o;
}


} // end anonymous namespace

/**
    Writes current state to file.  \returns true on success. */
bool
GaussianProcessEmulatorSingleFileWriter
::Write(GaussianProcessEmulator * gpe, std::ostream & output) const
{
  output.precision(17);
  serializeGaussianProcessEmulator(*gpe, output);

  return true;
}


/**
    Writes current state of PCADecomposition to file. */
bool
GaussianProcessEmulatorSingleFileWriter
::WritePCA(GaussianProcessEmulator * gpe, std::ostream & output) const
{
  output.precision(17);
  serializePCADecomposition(*gpe,output);

  return true;
}


/**
     Writes current state to file.  \returns true on sucess. */
bool
GaussianProcessEmulatorSingleFileWriter
::PrintThetas(GaussianProcessEmulator * gpe, std::ostream & output) const
{
  output.precision(17);
  serializeComments(gpe->m_Comments,output);
  output << "THETAS_FILE\n";
  output << "SUBMODELS\t"
    << gpe->m_NumberPCAOutputs << "\n\n";
  for (int i = 0; i < gpe->m_NumberPCAOutputs; ++i) {
    const GaussianProcessEmulator::SingleModel & m = gpe->m_PCADecomposedModels[i];
    output << "MODEL " << i << '\n';
    output << "COVARIANCE_FUNCTION\t"
      << GetCovarianceFunctionString(m.m_CovarianceFunction) << '\n';
    output << "REGRESSION_ORDER\t" << m.m_RegressionOrder << '\n';
    output << "THETAS\n";
    PrintVector(m.m_Thetas, output);
    output << "END_OF_MODEL\n\n";
  }
  output << "END_OF_FILE\n";
  return true;
}


} // end namespace madai
