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

#ifndef madai_Sampler_h_included
#define madai_Sampler_h_included

#include <set>
#include <string>

#include "Model.h"
#include "Parameter.h"
#include "Sample.h"


namespace madai {


class Model;


/** \class Sampler
 *
 * Base class for algorithms that sample from a distribution.
 */
class Sampler {
public:

  /** Error codes. */
  typedef enum {
    /** No error. */
    NO_ERROR = 0,

    /** Indicates that a parameter index is invalid. */
    INVALID_PARAMETER_INDEX_ERROR

  } ErrorType;

  Sampler();
  virtual ~Sampler();

  /**
   * Set the current Model
   *
   * \warning The observed values and covariances must be set prior to
   * calling this method. Use Model::SetObservedScalarValues() and
   * Model::SetObservedScalarCovariance() or call
   * Model::LoadObservations() to set these values.
   *
   * \param model The Model on which the Sampler will operate. */
  void SetModel( const Model * model );

  /**
   * Get the current Model.
   *
   * \return The Model in which the Sampler operates. */
  const Model * GetModel() const;

  /**
   * Get the number of parameters.
   *
   * \return The number of Parameters as defined by the Model from which
   *  this Sampler draws samples. */
  virtual unsigned int GetNumberOfParameters() const;

  /**
   * Get the list of parameters.
   *
   * These are not the parameter values but instead a description of
   * the parameter. */
  virtual const std::vector< Parameter > & GetParameters() const;

  /*
   * The following functions refer to active and inactive parameters.
   * Parameters may be refered to by name or by index.
   */

  /**
   * Get a list of the names of the active Parameters. */
  std::set< std::string > GetActiveParameters() const;

  /**
   * Which Parameters are active?  Indices correspond to indexes
   * within the vector returned by Sampler::GetParameters().  */
  const std::vector< bool > & GetActiveParametersByIndex() const;

  /**
   * Get the number of active Parameters. */
  unsigned int GetNumberOfActiveParameters() const;

  /**
   * Check whether the Parameter of the given name is active.
   *
   * \param parameterName Name of the Parameter. */
  bool IsParameterActive( const std::string & parameterName ) const;

  /**
   * Check whether the Parameter at the given index is active.
   *
   * \param parameterIndex The index of the Parameter.  This index
   * corresponds to the position of the Parameter in the vector of
   * Parameters returned by Sampler::GetParameters(). */
  bool IsParameterActive( unsigned int parameterIndex ) const;

  /**
   * Activate a Parameter by name
   *
   * Activating a Parameter means that the Parameter value will be
   * varied by the sampler.
   *
   * \param parameterName Name of the parameter to activate. */
  void ActivateParameter( const std::string & parameterName );

  /**
   * Activate a Parameter by index.
   *
   * Activating a Parameter means that the Parameter value will be
   * varied by the sampler.
   *
   * \param parameterIndex The index of the Parameter to
   * activate. This index corresponds to the position of the
   * Parameter in the vector of Parameters returned by
   * Sampler::GetParameters(). */
  void ActivateParameter( unsigned int parameterIndex );

  /**
   * Deactivate a parameter by name.
   *
   * Deactivating a Parameter means that the Parameter value will not
   * be varied by the sampler.
   *
   * \param parameterName Name of the Parameter to deactivate. */
  void DeactivateParameter( const std::string & parameterName );

  /**
   * Deactivate a parameter by index.
   *
   * Deactivating a Parameter means that the Parameter value will not
   * be varied by the Sampler.
   *
   * \param parameterIndex The index of the Parameter to
   * deactivate. This index corresponds to the position of the
   * Parameter in the vector of Parameters returned by
   * Sampler::GetParameters(). */
  void DeactivateParameter( unsigned int parameterIndex );


  /*
   * The Sampler stores a set of current values for the parameter.
   *
   * Samplers will change these values at each call to NextSample(),
   * but will not change a parameter values if
   * (! IsParameterActive(param)).
   */

  /**
   * Sets a Parameter value by name.
   *
   * \param parameterName The name of the Parameter whose value should
   *  be set.
   * \param value The new value of the Parameter.
   *
   * This method will trigger a call to
   * Sampler::ParameterSetExternally()  */
  virtual ErrorType SetParameterValue( const std::string & parameterName,
                                       double value );

  /**
   * Sets a Parameter value by index.
   *
   * \param parameterIndex  The index of the Parameter.  This index
   * corresponds to the position of the Parameter in the vector of
   * Parameters returned by Sampler::GetParameters().
   * \param value  The new value of the Parameter.
   *
   * This method will trigger a call to
   * Sampler::ParameterSetExternally() */
  virtual ErrorType SetParameterValue( unsigned int parameterIndex,
                                       double value );

  /**
   * Sets all of the Parameters at one time.
   *
   * \param parameterValues a vector of new parameters.  Must be
   * correct length.
   *
   * This method will trigger a call to
   * Sampler::ParameterSetExternally()  */
  virtual ErrorType SetParameterValues(
      const std::vector< double > & parameterValues);


  /**
   * Get the current value of a Parameter.
   *
   * \param parameterName The name of the Parameter whose value you
   *   want.
   * \return The value of the Parameter. */
  virtual double GetParameterValue( const std::string & parameterName ) const;

  /**
   * Get the current Parameter values.  */
  const std::vector< double > & GetCurrentParameters() const;

  /**
   * Return the Model's outputs for Sampler::GetCurrentParameters().  */
  const std::vector< double > & GetCurrentOutputs()  const;

  /**
   * Return the Model's log likelihood at Sampler::GetCurrentParameters(). */
  double GetCurrentLogLikelihood() const;

  /**
   * Compute the next set of parameters, output scalar values, and
   * log likelihood
   *
   * \return A new Sample. */
  virtual Sample NextSample() = 0;

  /**
   Return ErrorType as string. */
  static std::string GetErrorTypeAsString( ErrorType error );


protected:
  /**
   * Which Parameters are active?  Indices correspond to indices
   * within this->GetParameters().
   */
  std::vector< bool > m_ActiveParameterIndices;

  /** The Model from which this Sampler takes samples. */
  const Model * m_Model;

  /** Set of strings naming the active parameters. */
  std::set< std::string > m_ActiveParameters;

  /** Stores the point in parameter space. This will often change when
   *  NextSample() is called. */
  std::vector< double > m_CurrentParameters;

  /** Stores the outputs for the point in parameter space. This will
   * often change when NextSample() is called. */
  std::vector< double > m_CurrentOutputs;

  /** Stores the current log-likelihood. */
  double m_CurrentLogLikelihood;

  /* Protected methods: */

  /** Initialize the Sampler
   *
   * \param model The Model from which the Sampler should draw
   *  samples. */
  virtual void Initialize( const Model * model );

  /** Given the name of a Parameter, return its index
   *
   * \param parameterName Name of the Parameter for which the index is
   *   sought.
   * \return The index of the Parameter if found, the maximum value of
   *   unsigned int otherwise. */
  unsigned int GetParameterIndex( const std::string & parameterName ) const;

  /**
   * Subclasses that need to reset internal state when a parameter
   * value has been changed outside the operation of the optimization
   * algorithm should override this method. */
  virtual void ParameterSetExternally();

}; // end Sampler

} // end namespace madai

#endif // madai_Sampler_h_included
