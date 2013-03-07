/*=========================================================================
 *
 *  Copyright The University of North Carolina at Chapel Hill
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

#include "Model.h"

extern "C"{
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_math.h>
}
#include <cassert>
#include <cmath>

namespace madai {

Model
::Model() :
  m_GradientEstimateStepSize( 1.0e-4 ),
  m_StateFlag( UNINITIALIZED )
{
}


Model
::~Model()
{
}


Model::ErrorType
Model
::LoadConfigurationFile( const std::string fileName )
{
  return METHOD_NOT_IMPLEMENTED;
}


bool
Model
::IsReady() const
{
  return ( m_StateFlag == READY );
}


unsigned int
Model
::GetNumberOfParameters() const
{
  return static_cast<unsigned int>(m_Parameters.size());
}


const std::vector< Parameter > &
Model
::GetParameters() const
{
  return m_Parameters;
}


std::vector< std::string >
Model
::GetParameterNames() const
{
  std::vector< std::string > parameterNames;
  parameterNames.reserve( m_Parameters.size() );
  for ( std::vector< Parameter >::const_iterator iter = m_Parameters.begin();
        iter != m_Parameters.end();
        ++iter ) {
    parameterNames.push_back( iter->m_Name );
  }
}


unsigned int
Model
::GetNumberOfScalarOutputs() const
{
  return static_cast<unsigned int>(m_ScalarOutputNames.size());
}


Model::ErrorType
Model
::GetRange( unsigned int parameterIndex, double range[2] ) const
{
  if ( parameterIndex > this->GetNumberOfParameters() ) {
    return INVALID_PARAMETER_INDEX;
  }

  range[0] = this->m_Parameters.at(parameterIndex).m_MinimumPossibleValue;
  range[1] = this->m_Parameters.at(parameterIndex).m_MaximumPossibleValue;

  return NO_ERROR;
}


const std::vector< std::string > &
Model
::GetScalarOutputNames() const
{
  return m_ScalarOutputNames;
}


Model::ErrorType
Model
::GetScalarAndGradientOutputs(
  const std::vector< double > & parameters,
  const std::vector< bool > & activeParameters,
  std::vector< double > & scalars,
  unsigned int outputIndex,
  std::vector< double > & gradient) const
{
  if ( outputIndex >= this->GetNumberOfScalarOutputs() ) {
    return INVALID_OUTPUT_INDEX;
  }

  if ( static_cast< unsigned int >( activeParameters.size() ) !=
       this->GetNumberOfParameters() ) {
    return INVALID_ACTIVE_PARAMETERS;
  }

  // Make a copy of the parameters that we can work with
  std::vector< double > parametersCopy( parameters );

  // Clear the output vectors
  scalars.clear();
  gradient.clear();

  Model::ErrorType scalarOutputError;

  double h = m_GradientEstimateStepSize;
  for ( unsigned int i = 0; i < this->GetNumberOfParameters(); ++i ) {

    if ( activeParameters[i] ) {
      // Save the original parameter value
      double originalParameterValue = parametersCopy[i];

      // Compute the scalar outputs for a forward step
      parametersCopy[i] = parameters[i] + h;
      std::vector< double > forwardScalars;
      scalarOutputError = this->GetScalarOutputs(
        parametersCopy, forwardScalars );
      if ( scalarOutputError != NO_ERROR ) {
        return scalarOutputError;
      }

      // Compute the scalar outputs for a backward step
      parametersCopy[i] = parameters[i] - h;
      std::vector< double > backwardScalars;
      scalarOutputError = this->GetScalarOutputs(
        parametersCopy, backwardScalars );
      if ( scalarOutputError != NO_ERROR ) {
        return scalarOutputError;
      }

      // Compute the partial derivative with central differences
      double f = forwardScalars[ outputIndex ];
      double b = backwardScalars[ outputIndex ];
      double partialDerivative = ( f - b ) / ( 2.0 * h );

      // Store the partial derivative in the gradient output
      gradient.push_back( partialDerivative );

      // Restore the original parameter value
      parametersCopy[i] = originalParameterValue;
    }

  }

  // Now compute the scalars
  scalarOutputError = this->GetScalarOutputs( parameters, scalars );
  if ( scalarOutputError != NO_ERROR ) {
    return scalarOutputError;
  }

  return NO_ERROR;
}


void
Model
::SetGradientEstimateStepSize( double stepSize )
{
  m_GradientEstimateStepSize = stepSize;
}


double
Model
::GetGradientEstimateStepSize() const
{
  return m_GradientEstimateStepSize;
}


std::string
Model
::GetErrorTypeAsString( Model::ErrorType error )
{
  std::string outputString( "NO_ERROR" );

  switch ( error ) {

  case INVALID_PARAMETER_INDEX:
    outputString = std::string( "INVALID_PARAMETER_INDEX" );
    break;

  case INVALID_OUTPUT_INDEX:
    outputString = std::string( "INVALID_OUTPUT_INDEX" );
    break;

  case INVALID_ACTIVE_PARAMETERS:
    outputString = std::string( "INVALID_ACTIVE_PARAMETERS" );
    break;

  case FILE_NOT_FOUND_ERROR:
    outputString = std::string( "FILE_NOT_FOUND_ERROR" );
    break;

  case METHOD_NOT_IMPLEMENTED:
    outputString = std::string( "METHOD_NOT_IMPLEMENTED" );
    break;

  case WRONG_VECTOR_LENGTH:
    outputString = std::string( "WRONG_VECTOR_LENGTH" );
    break;

  case OTHER_ERROR:
    outputString = std::string( "OTHER_ERROR" );
    break;

  default:
    break;

  }

  return outputString;
}


void
Model
::AddParameter( const std::string & name,
                double minimumPossibleValue,
                double maximumPossibleValue )
{
  m_Parameters.push_back( Parameter(name, minimumPossibleValue, maximumPossibleValue) );
}

void
Model
::AddParameter( const std::string & name,
                const Distribution & priorDistribution)
{
  m_Parameters.push_back(
    Parameter(name, priorDistribution) );
}


void
Model
::AddScalarOutputName( const std::string & name )
{
  m_ScalarOutputNames.push_back( name );
}


Model::ErrorType
Model
::SetObservedScalarValues(const std::vector< double > & observedScalarValues)
{
  size_t size = observedScalarValues.size();
  if ((size != this->GetNumberOfScalarOutputs()) && (size != 0))
    return WRONG_VECTOR_LENGTH;
  // copy the vector
  this->m_ObservedScalarValues = observedScalarValues;
  return NO_ERROR;
}


Model::ErrorType
Model
::SetObservedScalarCovariance(
    const std::vector< double > & observedScalarCovariance)
{
  size_t size = observedScalarCovariance.size();
  if (size == 0) { // represents zero matrix.
    this->m_ObservedScalarCovariance.clear();
    return NO_ERROR;
  }
  size_t t = this->GetNumberOfScalarOutputs();
  if (size != (t * t))
    return WRONG_VECTOR_LENGTH;
  this->m_ObservedScalarCovariance = observedScalarCovariance;
  return NO_ERROR;
}


Model::ErrorType
Model
::GetScalarOutputsAndCovariance(
      const std::vector< double > & parameters,
      std::vector< double > & scalars,
      std::vector< double > & scalarCovariance)
{
  scalarCovariance.clear();
  return this->GetScalarOutputs(parameters, scalars);
}

Model::ErrorType
Model
::GetScalarOutputsAndLogLikelihood(
    const std::vector< double > & parameters,
    std::vector< double > & scalars,
    double & logLikelihood)
{
  logLikelihood = GSL_NAN; // if error occurs.
  double logPriorLikelihood
    = this->GetLogPriorLikelihood(parameters);

  size_t t = this->GetNumberOfScalarOutputs();
  assert(t > 0);
  std::vector< double > scalarCovariance;
  Model::ErrorType result = this->GetScalarOutputsAndCovariance(
    parameters, scalars, scalarCovariance);

  if (result != NO_ERROR)
    return result;
  if (scalars.size() != t)
    return OTHER_ERROR;

  std::vector< double > scalarDifferences(t);
  std::vector<double> covariance(t * t);

  double distSq = 0.0;
  if (this->m_ObservedScalarValues.size() == 0) {
    for (size_t i = 0; i < t; ++i) {
      scalarDifferences[i] = scalars[i];
      distSq += std::pow(scalarDifferences[i],2);
    }
  } else {
    for (size_t i = 0; i < t; ++i) {
      scalarDifferences[i] = scalars[i] - this->m_ObservedScalarValues[i];
      distSq += std::pow(scalarDifferences[i],2);
    }
  }

  if ((scalarCovariance.size() == 0) &&
      (this->m_ObservedScalarCovariance.size() == 0)) {
    // Infinite precision makes no sense, so assume variance of 1.0
    // for each variable.
    logLikelihood = ((-0.5) * distSq) + logPriorLikelihood;
    return NO_ERROR;
  } else if (scalarCovariance.size() == 0) {
    assert(this->m_ObservedScalarCovariance.size() == (t*t));
    covariance = this->m_ObservedScalarCovariance;
  } else if (this->m_ObservedScalarCovariance.size() == 0) {
    assert(scalarCovariance.size() == (t*t));
    covariance = scalarCovariance;
  } else {
    for (size_t i = 0; i < (t*t); ++i)
      covariance[i]
        = scalarCovariance[i] + this->m_ObservedScalarCovariance[i];
  }
  // We should replace this GSL code with some other linalg library.

  gsl_matrix_view cov_view
    = gsl_matrix_view_array(&(covariance[0]), t, t);
  gsl_matrix * lu = &cov_view.matrix;

  gsl_vector_view diff_view
    = gsl_vector_view_array (&(scalarDifferences[0]), t);
  gsl_vector * diff = &diff_view.vector;

  gsl_vector * tmp = gsl_vector_alloc(t);
  gsl_permutation * p = gsl_permutation_alloc(t);

  // Need to calculate ((covariance)^(-1) . differences)
  // tmp = (cov^(-1) . diff)
  // cov . tmp = diff
  // solve for tmp.
  int signum;
  gsl_linalg_LU_decomp (lu, p, &signum);
  //FIXME check for singular matrix
  gsl_linalg_LU_solve (lu, p, diff, tmp);
  // need to calulate ((diff)^T . tmp), the dot product of two vectors.
  double innerProduct;
  gsl_blas_ddot (diff, tmp, &innerProduct);

  gsl_vector_free(tmp);
  gsl_permutation_free(p);

  logLikelihood = ((-0.5) * innerProduct) + logPriorLikelihood;
  return NO_ERROR;
}



/** return the sum of the LogPriorLikelihood for each x[i] */
double
Model
::GetLogPriorLikelihood(const std::vector< double > & x) const
{
  const std::vector< Parameter > & params = this->GetParameters();
  assert(x.size() == params.size());
  double logPriorLikelihood = 0.0;
  for (size_t i = 0; i < params.size() ; ++i) {
    logPriorLikelihood +=
      params[i].GetPriorDistribution()->GetLogProbabilityDensity(x[i]);
  }
  return logPriorLikelihood;
}


} // end namespace madai



