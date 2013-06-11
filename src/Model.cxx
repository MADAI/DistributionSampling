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

#include "Model.h"

#include <Eigen/Dense>

#include <cassert>
#include <cmath>
#include <limits>

template<class S, class T>
int static findIndex(const S & v, const T & s)
{
  typename S::const_iterator it = std::find(v.begin(), v.end(), s);
  if (it == v.end())
    return -1;
  return std::distance(v.begin(), it);
}

namespace madai {

Model
::Model() :
  m_GradientEstimateStepSize( 1.0e-4 ),
  m_StateFlag( UNINITIALIZED ),
  m_UseModelCovarianceToCalulateLogLikelihood( false )
{
}


Model
::~Model()
{
}


Model::ErrorType
Model
::LoadConfigurationFile( const std::string )
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
  return parameterNames;
}


unsigned int
Model
::GetNumberOfScalarOutputs() const
{
  return static_cast<unsigned int>(m_ScalarOutputNames.size());
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
  std::vector< double > & gradient) const
{
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
      double forwardLogLikelihood;
      scalarOutputError = this->GetScalarOutputsAndLogLikelihood(
        parametersCopy, forwardScalars, forwardLogLikelihood );
      if ( scalarOutputError != NO_ERROR ) {
        return scalarOutputError;
      }

      // Compute the scalar outputs for a backward step
      parametersCopy[i] = parameters[i] - h;
      std::vector< double > backwardScalars;
      double backwardLogLikelihood;
      scalarOutputError = this->GetScalarOutputsAndLogLikelihood(
        parametersCopy, backwardScalars, backwardLogLikelihood );
      if ( scalarOutputError != NO_ERROR ) {
        return scalarOutputError;
      }

      // Compute the partial derivative with central differences
      double partialDerivative = ( forwardLogLikelihood - backwardLogLikelihood )
        / ( 2.0 * h );

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


bool
Model
::GetUseModelCovarianceToCalulateLogLikelihood()
{
  return m_UseModelCovarianceToCalulateLogLikelihood;
}


void
Model
::SetUseModelCovarianceToCalulateLogLikelihood(bool v)
{
  m_UseModelCovarianceToCalulateLogLikelihood = v;
}


/**
   Load a file with experimental observations in it.  The model will
   be comared against this. */
Model::ErrorType
Model
::LoadObservations(std::istream & i)
{
  // std::ifstream i("DIRECTORY/experimental_results/results.dat");
  const std::vector< std::string > & scalarOutputNames = this->GetScalarOutputNames();
  unsigned int numberOfScalarOutputs = this->GetNumberOfScalarOutputs();
  assert(scalarOutputNames.size() == numberOfScalarOutputs);
  assert (numberOfScalarOutputs > 0);
  std::vector< double > observedScalarValues(numberOfScalarOutputs, 0.0);
  std::vector< double > observedScalarCovariance(
      numberOfScalarOutputs * numberOfScalarOutputs, 0.0);
  for (unsigned int j = 0; j < numberOfScalarOutputs; ++j)
    observedScalarCovariance[j * (1 + numberOfScalarOutputs)] = 1.0;
  while (true) { // will loop forever if input stream lasts forever.
    std::string name;
    double value, uncertainty;
    if(! (i >> name >> value >> uncertainty))
      break;
    int index = findIndex(scalarOutputNames, name);
    if (index != -1) {
      observedScalarValues[index] = value;
      // observedScalarCovariance is a square matrix;
      observedScalarCovariance[index * (1 + numberOfScalarOutputs)] = std::pow(uncertainty, 2);
      // uncertainty^2 is variance.
    }
  }
  // assume extra values are all zero.
  Model::ErrorType e;
  e = this->SetObservedScalarValues(observedScalarValues);
  if (e != madai::Model::NO_ERROR) {
    std::cerr << "Error in Model::SetObservedScalarValues\n";
    return e;
  }
  e = this->SetObservedScalarCovariance(observedScalarCovariance);
  if (e != madai::Model::NO_ERROR) {
    std::cerr << "Error in Model::SetObservedScalarCovariance\n";
    return e;
  }
  return madai::Model::NO_ERROR;
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
      std::vector< double > & scalarCovariance) const
{
  scalarCovariance.clear();
  return this->GetScalarOutputs(parameters, scalars);
}

Model::ErrorType
Model
::GetScalarOutputsAndLogLikelihood(
    const std::vector< double > & parameters,
    std::vector< double > & scalars,
    double & logLikelihood) const
{


  logLikelihood = std::numeric_limits< double >::signaling_NaN();
  double logPriorLikelihood
    = this->GetLogPriorLikelihood(parameters);

  size_t t = this->GetNumberOfScalarOutputs();
  assert(t > 0);
  std::vector< double > scalarCovariance;
  // initially scalarCovariance is a empty vector
  Model::ErrorType result;
  if (m_UseModelCovarianceToCalulateLogLikelihood) {
    result = this->GetScalarOutputsAndCovariance(
        parameters, scalars, scalarCovariance);
  } else {
    // (! m_UseModelCovarianceToCalulateLogLikelihood)
    result = this->GetScalarOutputs(parameters, scalars);
  }
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

  std::vector< double > constantCovariance;
  if ( !this->GetConstantCovariance(constantCovariance) ) {
    std::cerr << "Error getting the constant covariance matrix from the model\n";
    return OTHER_ERROR;
  }

  if ((scalarCovariance.size() == 0) &&
      (constantCovariance.size() == 0)) {
    // Infinite precision makes no sense, so assume variance of 1.0
    // for each variable.
    logLikelihood = ((-0.5) * distSq) + logPriorLikelihood;
    return NO_ERROR;
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

  Eigen::Map< Eigen::VectorXd > diff(&(scalarDifferences[0]),t);
  Eigen::Map< Eigen::MatrixXd > cov(&(covariance[0]),t,t);

  // FIXME check for singular matrix -> return negative infinity!
  //assert( cov.determinant() >= 0.0 ); // is there a better way?

  double innerProduct = cov.colPivHouseholderQr().solve(diff).dot(diff);
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
  for ( size_t i = 0; i < params.size(); ++i ) {
    logPriorLikelihood +=
      params[i].GetPriorDistribution()->GetLogProbabilityDensity(x[i]);
  }
  return logPriorLikelihood;
}


/** return the gradient of the LogPriorLikelihood at x */
std::vector< double >
Model
::GetGradientOfLogPriorLikelihood(
  const std::vector< double > & x) const
{
  const std::vector< Parameter > & params = this->GetParameters();
  assert(x.size() == params.size());
  std::vector< double > gradient;
  for ( size_t i = 0; i < params.size(); i++ ) {
    gradient.push_back(
      params[i].GetPriorDistribution()->GetGradientLogProbabilityDensity( x[i]) );
  }
  return gradient;
}


bool
Model
::GetConstantCovariance(std::vector< double > & x) const
{
  x.clear();
  unsigned int t = m_ScalarOutputNames.size();
  if ( m_ObservedScalarCovariance.size() != (t*t) ) {
    std::cerr << "Observed scalar covariance is of invalid size "
              << m_ObservedScalarCovariance.size() << "\n";
    return false;
  }

  x.resize(t*t);
  x = m_ObservedScalarCovariance;
  return true;
}


} // end namespace madai
