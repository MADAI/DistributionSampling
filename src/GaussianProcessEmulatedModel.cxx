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

#include "GaussianProcessEmulatedModel.h"
#include "GaussianProcessEmulatorDirectoryFormatIO.h"
#include "GaussianProcessEmulator.h"

namespace madai {


GaussianProcessEmulatedModel
::GaussianProcessEmulatedModel() :
  m_GPE( new GaussianProcessEmulator )
{
  m_StateFlag = UNINITIALIZED;
}

GaussianProcessEmulatedModel
::~GaussianProcessEmulatedModel()
{
  delete m_GPE;
}


/**
 * Loads a configuration from a directory structure. The format of the file
 * is defined by this function.
 */
Model::ErrorType
GaussianProcessEmulatedModel
::LoadConfiguration( const std::string StatisticsDirectory,
                     const std::string ModelOutputDirectory,
                     const std::string ExperimentalResultsDirectory )
{
  GaussianProcessEmulatorDirectoryFormatIO directoryReader;
  if ( !directoryReader.LoadTrainingData( m_GPE, ModelOutputDirectory,
                                          StatisticsDirectory, ExperimentalResultsDirectory ) ) {
    std::cerr << "Error loading from the directory structure.\n";
    return Model::OTHER_ERROR;
  }
  if ( !directoryReader.LoadPCA( m_GPE, StatisticsDirectory ) ) {
    std::cerr << "Error loading the PCA decomposition data.\n";
    return Model::OTHER_ERROR;
  }
  if ( !directoryReader.LoadEmulator( m_GPE, StatisticsDirectory ) ) {
    std::cerr << "Error loading Emulator data.\n";
    return Model::OTHER_ERROR;
  }

  if ( m_GPE->m_Status != GaussianProcessEmulator::READY )
    return Model::OTHER_ERROR;
  m_StateFlag = READY;
  m_Parameters = m_GPE->m_Parameters;
  m_ScalarOutputNames = m_GPE->m_OutputNames;
  if ( !m_GPE->GetUncertaintyScalesAsCovariance( m_TrainingAndObservedCovariance ) ) {
    std::cerr << "Error setting the covariance containing experimental and model"
              << " output data.\n";
    return Model::OTHER_ERROR;
  }
  return Model::NO_ERROR;
}

/**
 * Set the gaussian process emulator
 */
Model::ErrorType
GaussianProcessEmulatedModel
::SetGaussianProcessEmulator(
  GaussianProcessEmulator & GPME )
{
  *m_GPE = GPME; // Copy entire object, not pointer.  Makes tracking ownership easy.
  if ( m_GPE->m_Status != GaussianProcessEmulator::READY )
    return Model::OTHER_ERROR;
  m_StateFlag = READY;
  m_Parameters = m_GPE->m_Parameters;
  m_ScalarOutputNames = m_GPE->m_OutputNames;
  if ( !m_GPE->GetUncertaintyScalesAsCovariance( m_TrainingAndObservedCovariance ) ) {
    std::cerr << "Error setting the covariance containing experimental and model"
              << " output data.\n";
    return Model::OTHER_ERROR;
  }
  return Model::NO_ERROR;
}

/**
   Returns a const reference to internal data for debugging purposes. */
const GaussianProcessEmulator &
GaussianProcessEmulatedModel
::GetGaussianProcessEmulator() const
{
  return *m_GPE;
}

/**
 * Get the scalar outputs from the model evaluated at x.  If an
 * error happens, the scalar output array will be left incomplete.
 */
Model::ErrorType
GaussianProcessEmulatedModel
::GetScalarOutputsAndCovariance(
  const std::vector< double > & parameters,
  std::vector< double > & scalars,
  std::vector< double > & scalarCovariance) const
{
  if (m_GPE->m_Status != GaussianProcessEmulator::READY)
    return Model::OTHER_ERROR;

  if (this->GetNumberOfParameters() != parameters.size())
    return Model::OTHER_ERROR;

  if (! m_GPE->GetEmulatorOutputsAndCovariance(
          parameters, scalars, scalarCovariance))
    return Model::OTHER_ERROR;

  return Model::NO_ERROR;
}

/**
 * Get the scalar outputs from the model evaluated at a position in
 * the parameter space.
 */
Model::ErrorType
GaussianProcessEmulatedModel
::GetScalarOutputs(
  const std::vector< double > & parameters,
  std::vector< double > & scalars ) const
{
  if (m_GPE->m_Status != GaussianProcessEmulator::READY)
    return Model::OTHER_ERROR;

  if (this->GetNumberOfParameters() != parameters.size())
    return Model::OTHER_ERROR;

  if (! m_GPE->GetEmulatorOutputs (parameters, scalars))
    return Model::OTHER_ERROR;

  return Model::NO_ERROR;
}

// Overwirte numeric gradient with analytic gradient
Model::ErrorType
GaussianProcessEmulatedModel
::GetScalarAndGradientOutputs(
  const std::vector< double > & parameters,
  const std::vector< bool > & activeParameters,
  std::vector< double > & scalars,
  std::vector< double > & gradient ) const
{
  if ( static_cast< unsigned int >( activeParameters.size() ) !=
      this->GetNumberOfParameters() ) {
    return INVALID_ACTIVE_PARAMETERS;
  }

  // Make a copy of the parameters that we can work with
  std::vector< double > parametersCopy( parameters );

  // Clear the output vector
  gradient.clear();

  // Get the gradient of the model outputs
  std::vector< double > meanGradients;
  std::vector< Eigen::MatrixXd > covarianceGradients;
  if ( m_GPE->m_Status != GaussianProcessEmulator::READY ) {
    std::cerr << "Error: Emulator not ready.\n";
    return Model::OTHER_ERROR;
  }
  if ( this->GetNumberOfParameters() != parameters.size() ) {
    std::cerr << "Error: Size of parameters vector is invalid with '" << parameters.size()
              << "' parameters, it should be of size '" << this->GetNumberOfParameters() << "\n";
    return Model::OTHER_ERROR;
  }
  if ( m_UseModelCovarianceToCalulateLogLikelihood )
    if ( !m_GPE->GetGradientsOfCovariances( parameters, covarianceGradients ) ) {
      std::cerr << "Error in GaussianProcessEmulator::GetGradientsOfCovariances.\n";
      return Model::OTHER_ERROR;
    }
  if ( !m_GPE->GetGradientOfEmulatorOutputs( parameters, meanGradients ) ) {
    std::cerr << "Error in GaussianProcessEmulator::GetGradientOfEmulatorOutputs.\n";
    return Model::NO_ERROR;
  }

  int p = parameters.size();
  size_t t = this->GetNumberOfScalarOutputs();
  assert( t > 0 );
  std::vector< double > scalarCovariance;
  std::vector< double > covariance( t * t );
  Model::ErrorType result;
  if ( m_UseModelCovarianceToCalulateLogLikelihood ) {
    result = this->GetScalarOutputsAndCovariance(
        parameters, scalars, scalarCovariance );
  } else {
    result = this->GetScalarOutputs( parameters, scalars );
  }
  if ( result != Model::NO_ERROR )
    return result;
  if ( scalars.size() != t )
    return Model::OTHER_ERROR;

  std::vector< double > scalarDifferences(t);
  if ( m_ObservedScalarValues.size() == 0 ) {
    for ( size_t i = 0; i < t; ++i ) {
      scalarDifferences[i] = scalars[i];
    }
  } else {
    for ( size_t i = 0; i < t; ++i ) {
      scalarDifferences[i] = scalars[i] - m_ObservedScalarValues[i];
    }
  }

  // Get the constant covariance matrix
  std::vector< double > constantCovariance;
  if ( !this->GetConstantCovariance(constantCovariance) ) {
    std::cerr << "Error getting the constant covariance matrix from the model.\n";
    return Model::OTHER_ERROR;
  }

  if ( scalarCovariance.size() == 0 &&
      constantCovariance.size() == 0 ) {
    // Infinite precision makes no sense, so assume variance of 1.0
    // for each variable. Set covariance to Identity.
    covariance.resize(t*t);
    for ( unsigned int i = 0; i < (t*t); i++ )
      covariance[i] = 0.0;
    for ( unsigned int i = 0; i < t; i++ )
      covariance[i*(t+1)] = 1.0;
  } else if (scalarCovariance.size() == 0) {
    assert(constantCovariance.size() == (t*t));
    covariance = constantCovariance;
  } else if (constantCovariance.size() == 0) {
    assert(scalarCovariance.size() == (t*t));
    covariance = scalarCovariance;
  } else {
    for (size_t i = 0; i < (t*t); ++i)
      covariance[i]
        = scalarCovariance[i] + constantCovariance[i];
  }

  // Get gradient of the log prior likelihood
  std::vector< double > LPGradient
  = this->GetGradientOfLogPriorLikelihood( parameters );

  Eigen::Map< Eigen::VectorXd > diff(&(scalarDifferences[0]),t);
  Eigen::Map< Eigen::MatrixXd > cov(&(covariance[0]),t,t);
  Eigen::Map< Eigen::MatrixXd > MGrads(&(meanGradients[0]),t,p);
  Eigen::VectorXd LLGrad( p );
  Eigen::VectorXd t1( t );

  if ( scalarCovariance.size() == 0 &&
      constantCovariance.size() == 0 ) {
    // Assume variance of 1.0 for each output
    t1 = diff;
  } else {
    // FIXME check for singular matrix -> return negative infinity!
    //assert( cov.determinant() >= 0.0 ); // is there a better way?

    t1 = cov.colPivHouseholderQr().solve(diff);
  }

  LLGrad = -MGrads.transpose()*t1;
  if ( scalarCovariance.size() == 0 ) {
    // emulator error not used, do nothing
  } else {
    // Need to include derivative of covariance matrix
    for ( int i = 0; i < p; i++ ) {
      if ( activeParameters[i] ) {
        LLGrad(i) += -0.5*t1.dot(covarianceGradients[i]*t1);
      }
    }
  }

  for ( int i = 0; i < p; i++ ) {
    if ( activeParameters[i] ) {
      gradient.push_back( LLGrad(i) + LPGradient[i] );
    }
  }

  return NO_ERROR;
}

bool
GaussianProcessEmulatedModel
::GetConstantCovariance(std::vector< double > & x)  const
{
  x.clear();
  unsigned int t = m_ScalarOutputNames.size();
  if ( m_TrainingAndObservedCovariance.size() != (t*t) ) {
    std::cerr << "Constant covariance matrix has invalid size "
              << m_TrainingAndObservedCovariance.size() << "\n";
    return false;
  }

  x.resize(t*t);
  x = m_TrainingAndObservedCovariance;
  return true;
}

} // namespace madai
