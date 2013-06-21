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

#ifndef madai_Model_h_included
#define madai_Model_h_included

#include <cfloat>
#include <vector>
#include <iostream>

#include "Parameter.h"
#include "Random.h"

/** Namespace for all Distribution Sampling library classes. */
namespace madai {

/** \class Model
 *
 * Base class for Models. A Model's primary function is to compute
 * model values from a point in the Model's parameter space. In
 * addition, the log likelihood that the Model's scalar values match
 * the observed values from the system being modeled can be computed. */
class Model {
public:

  /** Error codes returned by various methods. */
  typedef enum {
    /** No error */
    NO_ERROR = 0,

    /** An invalid parameter was passed as an argument. */
    INVALID_PARAMETER_INDEX,

    /** The set of active parameters is invalid. */
    INVALID_ACTIVE_PARAMETERS,

    /** A file was not found. */
    FILE_NOT_FOUND_ERROR,

    /** A method was not implemented. */
    METHOD_NOT_IMPLEMENTED,

    /** A vector argument was not the expected length. */
    WRONG_VECTOR_LENGTH,

    /** A unknown error occured. */
    OTHER_ERROR
  } ErrorType;

  Model();
  virtual ~Model();

  /** Has the model been initialized?
   *
   * \return Returns true if the Model has been initialized, false
   * otherwise. */
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

  /** Get the scalar outputs from the model evaluated at x
   *
   * \param parameters Parameter values where the Model should be
   * evaluated. The length of this vector must equal the number of the
   * parameters in the Model.
   * \param scalars A vector to hold the scalar outputs from the Model
   * when evaluated at the point in parameter space specified by the
   * parameters argument. */
  virtual ErrorType GetScalarOutputs( const std::vector< double > & parameters,
                                      std::vector< double > & scalars ) const = 0;

  /** Get the gradient outputs from the priors evaluated at x.
   *
   * Outputs a vector containing the gradients of each parameter.
   *
   * \param x Point in parameter space where the gradient of the
   * LogPriorLikelihood will be evaluated.
   */
  std::vector< double > GetGradientOfLogPriorLikelihood(
    const std::vector< double > & x ) const;

  /** Get both scalar outputs and the gradient of active parameters.
   *
   * The gradient output parameter will contain gradient components of
   * the log likelihood for only the active parameters. That is, the
   * first element in the vector will contain the gradient component
   * for the first active parameter, the second element in the vector
   * will contain the gradient component for the second active
   * parameter, and so on.
   *
   * \param parameters Point in parameter space where the Model should
   * be evaluated.
   * \param activeParameters List of parameters for which the gradient
   * should be computed.
   * \param scalars Output argument that will contain the scalars from
   * evaluating the Model.
   * \param gradient Output argument that will contain the gradient
   * components requested via the activeParameters vector. */
  virtual ErrorType GetScalarAndGradientOutputs(
    const std::vector< double > & parameters,
    const std::vector< bool > & activeParameters,
    std::vector< double > & scalars,
    std::vector< double > & gradient) const;

  /** Expect vector of length GetNumberOfScalarOutputs().
   *
   * If you never set this, all model scalar outputs are assumed to
   * already have the observed value subtracted off.  By default, we
   * use the zero vector as ObservedScalarValues.  To unset this
   * value, pass a zero-length vector, which is interpreted as the
   * zero vector.
   */
  virtual ErrorType SetObservedScalarValues(
    const std::vector< double > & observedScalarValues);

  /** Expects a t-by-t symmetric covariance matrix, flattened into a
   * vector of length (t*t), where t = GetNumberOfScalarOutputs()
   *
   * This matrix represents the variances in the field measurements of
   * the observed scalars.  Consequently, the "distance" in output
   * space is based on the inverse of the covariance (the precision
   * matrix).
   *
   * If you never set this, assumes zero.  To calculate
   * log-likelihood, either the observed value, the model outputs, or
   * both MUST have a covariance value.
   */
  virtual ErrorType SetObservedScalarCovariance(
    const std::vector< double > & observedScalarCovariance);

  /** Gets the scalar outputs and log-likelihood of the model for a
   * point in parameter space
   *
   * 1) Calculates all of the scalar values at this point in parameter
   * space.
   * 2) calculates log-likelihood, using
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
   * and the log-likelihood will be negative-infinity. */
   virtual ErrorType GetScalarOutputsAndLogLikelihood(
    const std::vector< double > & parameters,
    std::vector< double > & scalars,
    double & logLikelihood) const;

  /** Some models don't know the output values precisely
   *
   * Instead they produce a distribution of possible output values,
   * with a mean and covariance.  In that case, this function should
   * be overridden by subclasses to return those means and that
   * covariance matrix (flattened into a vector).
   *
   * If not overridden, this will simply call GetScalarOutputs() and
   * return an empty vector for scalarCovariance, representing a zero
   * matrix.
   *
   * \param parameters Parameter values where the model should be evaluated.
   * \param scalars    Storage for scalar values returned by this method.
   * \param scalarCovariance Storage for the covariance of the model
   *                         at this point in parameter space.
   */
  virtual ErrorType GetScalarOutputsAndCovariance(
    const std::vector< double > & parameters,
    std::vector< double > & scalars,
    std::vector< double > & scalarCovariance) const;

  /**
   * Returns the sum of the LogPriorLikelihood for each x[i] which is
   * the sum of log of the prior likelihood for the parameters */
  virtual double GetLogPriorLikelihood(const std::vector< double > & x) const;

  /** Set the gradient estimate step size used in GetScalarAndGradientOutputs */
  void SetGradientEstimateStepSize( double stepSize );

  /** Get the gradient estimate step size. */
  double GetGradientEstimateStepSize() const;

  /** Returns an error code as a string. */
  static std::string GetErrorTypeAsString( ErrorType error );

  //@{
  /**
   * Should GetScalarOutputsAndLogLikelihood() call
   * GetScalarOutputsAndCovariance() or just GetScalarOutputs()?
   *
   * Defaults to not using the model covariance in the log likelihood
   * calculation.
   */
  bool GetUseModelCovarianceToCalulateLogLikelihood();
  void SetUseModelCovarianceToCalulateLogLikelihood(bool);
  //@}

  /** Returns the constant convariance, which is the covariance of the
   *  observed values by default. */
  virtual bool GetConstantCovariance(std::vector< double > & x) const;

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

  /**
   * Should GetScalarOutputsAndLogLikelihood() call
   * GetScalarOutputsAndCovariance() or just GetScalarOutputs()?
   */
  bool m_UseModelCovarianceToCalulateLogLikelihood;

  /** Add a parameter.
   *
   * \deprecated Use the priorDistribution form of this command.
   */
  void AddParameter( const std::string & name,
                     double minimumPossibleValue = -DBL_MAX,
                     double maximumPossibleValue =  DBL_MAX );

  /** Add a parameter. */
  void AddParameter( const std::string & name,
                     const Distribution & priorDistribution);

  /** Add a scalar output name. */
  void AddScalarOutputName( const std::string & name );


  /** A GetNumberOfScalarOutputs()-length vector.
   * if empty, assume zero vector */
  std::vector< double > m_ObservedScalarValues;

  /** A (GetNumberOfScalarOutputs x GetNumberOfScalarOutputs) matrix,
   * flattened so that we can use a gsl_matrix_view to look at it
   *
   * If empty, assume zero matrix. */
  std::vector< double > m_ObservedScalarCovariance;

}; // end Model

} // end namespace madai

#endif // madai_Model_h_included
