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

#ifndef __Model_h__
#define __Model_h__

#include "Parameter.h"
#include "parametermap.h"
#include "random.h"
#include "ScalarFunction.h"

extern "C"{
#include <gsl/gsl_randist.h>
}

#include <cfloat>
#include <vector>

namespace madai {

class Model {
public:
  typedef enum {
    NO_ERROR = 0,
    INVALID_PARAMETER_INDEX,
    INVALID_OUTPUT_INDEX,
    INVALID_ACTIVE_PARAMETERS,
    FILE_NOT_FOUND_ERROR,
    METHOD_NOT_IMPLEMENTED,
    WRONG_VECTOR_LENGTH,
    OTHER_ERROR
  } ErrorType;

  Model();
  virtual ~Model();

  /** Loads a configuration from a file. Subclasses should override
   * this method as this implementation does nothing. **/
  virtual ErrorType LoadConfigurationFile( const std::string fileName );

  /** Has the model been initialized? */
  bool IsReady() const;

  /** Get the number of parameters. */
  virtual unsigned int GetNumberOfParameters() const;

  /** Get names of the parameters. */
  virtual const std::vector< Parameter > & GetParameters() const;

  /** Get the names of the parameters. */
  virtual std::vector< std::string > GetParameterNames() const;

  /** Get the number of scalar outputs. */
  virtual unsigned int GetNumberOfScalarOutputs() const;

  /** Get the names of the scalar outputs of the model. */
  virtual const std::vector< std::string > & GetScalarOutputNames() const;

  /** Get the valid range for the parameter at parameterIndex. */
  virtual ErrorType GetRange( unsigned int parameterIndex, double range[2] ) const;

  /** Get the scalar outputs from the model evaluated at x. */
  virtual ErrorType GetScalarOutputs( const std::vector< double > & parameters,
                                      std::vector< double > & scalars ) const = 0;

  /** Get both scalar outputs and the gradient of active parameters.
   *
   * The gradient output parameter will contain gradient components of
   * the output scalar field requested by the outputIndex parameter
   * for only the active parameters. That is, the first element in the
   * vector will contain the gradient component for the first active
   * parameter, the second element in the vector will contain the
   * gradient component for the second active parameter, and so on. */
  virtual ErrorType GetScalarAndGradientOutputs(
    const std::vector< double > & parameters,
    const std::vector< bool > & activeParameters,
    std::vector< double > & scalars,
    unsigned int outputIndex,
    std::vector< double > & gradient) const;

  /** Get the likelihood and prior for the parameters. */
  virtual ErrorType GetLikeAndPrior( const std::vector< double > & parameters,
                                     double & LikeNew,
                                     double & PriorNew) const = 0;

  /**
   * Expect vector of length GetNumberOfScalarOutputs().  If you never
   * set this, all model scalar outputs are assumed to already have
   * the observed value subtracted off.  By default, we use the zero
   * vector as ObservedScalarValues.  To unset this value, pass a
   * zero-length vector, which is interpreted as the zero vector.
   */
  virtual ErrorType SetObservedScalarValues(
    const std::vector< double > & observedScalarValues);

  /**
   * Expects a t-by-t symmetric covariance matrix, flattened into a
   * vector of length (t*t), where t = GetNumberOfScalarOutputs().
   * This matrix represents the variances in the field measurements of
   * the observed scalars.  Consequently, the "distance" in output
   * space is based on the inverse of the covariance (the precision
   * matrix).
   *
   *  If you never set this, assumes zero.  To calculate
   *  log-likelihood, either the observed value, the model outputs, or
   *  both MUST have a covariance value.
   */
  virtual ErrorType SetObservedScalarCovariance(
    const std::vector< double > & observedScalarCovariance);

  /**
   * 1) Calculate all of the scalar values at this point in parameter
   * space.
   * 2) calculate log-likelihood, using
   *     model scalars outputs
   *     model scalar output covariance
   *     observed scalar values
   *     observed scalar covariance
   *
   * If not overridden, this function calls
   * GetScalarOutputsAndCovariance() and uses observedScalarValues and
   * and observedScalarCovariance to calculate LogLikelihood.  The
   * log-likelihood is returned along with the output scalars.  If both
   * covariances are present, they are summed.
   *
   * (scalars, scalarCovariance) = GetScalarOutputsAndCovariance(parameters)
   * logPriorLikelihood = LogPriorLikelihoodFunction(parameters)
   * covariance = observedScalarCovariance + scalarCovariance
   * differences = scalars - observedScalars
   * LogLikelihood = -0.5 * (differences^T . (covariance)^(-1) . differences);
   * @return logPriorLikelihood + LogLikelihood
   *
   * If both covariances are zero, the matrix will not be invertable
   * and the log-likelihood will be negative-infinity.
   *
   * If LogPriorLikelihoodFunction is NULL, it is assumed to be zero.
   */
  virtual ErrorType GetScalarOutputsAndLogLikelihood(
    const std::vector< double > & parameters,
    std::vector< double > & scalars,
    double & logLikelihood);

  /**
   * Some models don't know the output values precisely, instead they
   * produce a distribution of possible output values, with a mean and
   * covariance.  In that case, this function should be overridden by
   * subclasses to return those means and that covariance matrix
   * (flattened into a vector).
   *
   * If not overridden, this will simply call GetScalarOutputs() and
   * return an empty vector for scalarCovariance, representing a zero
   * matrix.
   */
  virtual ErrorType GetScalarOutputsAndCovariance(
    const std::vector< double > & parameters,
    std::vector< double > & scalars,
    std::vector< double > & scalarCovariance);

  /**
   * If this function is not set, assume a constant prior.
   * To unset this, call with NULL as the argument.
   *
   */
  virtual ErrorType SetLogPriorLikelihoodFunction( ScalarFunction * function );

  /** Set/get the gradient estimate step size. */
  void SetGradientEstimateStepSize( double stepSize );
  double GetGradientEstimateStepSize() const;

  /** Returns an error code as a string. */
  static std::string GetErrorTypeAsString( ErrorType error );

  /** \todo - These should be made protected. */
  std::string   m_DirectoryName;
  std::string   m_ParameterFileName;
  bool          m_LogLike;
  parameterMap  m_ParameterMap;

protected:
  /** Enumeration of internal state. */
  typedef enum {
    UNINITIALIZED,
    READY,
    ERROR
  } InternalState;

  /** Subclasses must populate this vector with the names of the
   * model parameters. */
  std::vector< Parameter > m_Parameters;

  /** Subclasses must populate these vectors with the names of the
   * scalar outputs. */
  std::vector< std::string > m_ScalarOutputNames;

  /** Step size used for numerical estimation of gradient.
   *
   * This is usually denoted 'h' in math texts about finite differences. */
  double m_GradientEstimateStepSize;

  /** Current state of the model. */
  InternalState m_StateFlag;

  /** Add a parameter. */
  void AddParameter( const std::string & name,
                     double minimumPossibleValue = -DBL_MAX,
                     double maximumPossibleValue =  DBL_MAX );

  /** Add a scalar output name. */
  void AddScalarOutputName( const std::string & name );


  /**
   * A GetNumberOfScalarOutputs()-length vector.
   * if empty, assume zero vector.
   */
  std::vector< double > m_ObservedScalarValues;
  /**
   * A (GetNumberOfScalarOutputs x GetNumberOfScalarOutputs) matrix,
   * flattened so that we can use a gsl_matrix_view to look at it.
   * If empty, assume zero matrix.
   */
  std::vector< double > m_ObservedScalarCovariance;
  /**
   * A function to get the LogPriorLikelihood at any point in
   * parameter space.
   */
  ScalarFunction * m_LogPriorLikelihoodFunction;

}; // end Model

} // end namespace madai

#endif // __Model_h
