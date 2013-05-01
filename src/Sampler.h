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

#include "Model.h"
#include "Trace.h"
#include "Parameter.h"
#include "Sample.h"


namespace madai {


class Model;


/** \class Sampler
 *
 * Makes a distribution proportional to exp(LogPosteriorLikelihood)
 * Base class for algorithms that sample from a distribution.
 */
class Sampler {
public:
  /** ErrorType */
  typedef enum {
    NO_ERROR = 0,
    INVALID_PARAMETER_INDEX_ERROR,
    INVALID_OUTPUT_SCALAR_INDEX_ERROR
  } ErrorType;

  /** Default constructor. */
  Sampler();

  /** Destructor. */
  virtual ~Sampler();

  /** Set the current Model
   *
   * \param model The Model on which the Sampler will operate. */
  void SetModel( const Model * model );

  /** Get the current Model
   *
   * \return The Model in which the Sampler operates. */
  const Model * GetModel() const;

  /** Get a list of the names of the active parameters. */
  std::set< std::string > GetActiveParameters();

  /** Activate a Parameter by name
   *
   * Activating a Parameter means that the Parameter value will be
   * varied by the sampler.
   *
   * \param parameterName Name of the parameter to activate. */
  void ActivateParameter( const std::string & parameterName );

  /** Activate a Parameter by index
   *
   * Activating a Parameter means that the Parameter value will be
   * varied by the sampler.
   *
   * \param parameterIndex The index of the Parameter to
   * activate. This index corresponds to the position of the Parameter
   * in the vector of Parameters returned by Sampler::GetParameters().
   */
  void ActivateParameter( unsigned int parameterIndex );

  /** Deactivate a parameter by name
   *
   * Deactivating a Parameter means that the Parameter value will not be
   * varied by the sampler.
   *
   * \param parameterName Name of the Parameter to deactivate. */
  void DeactivateParameter( const std::string & parameterName );

  /**
   * Deactivate a parameter by index
   *
   * Deactivating a Parameter means that the Parameter value will not be
   * varied by the sampler.
   *
   * \param parameterIndex The index of the Parameter to
   * deactivate. This index corresponds to the position of the Parameter
   * in the vector of Parameters returned by Sampler::GetParameters().
   */
  void DeactivateParameter( unsigned int parameterIndex );

  /** Get the number of parameters
   *
   * \return The number of Parameters as defined by the Model from which
   *  this Sampler draws samples. */
  virtual unsigned int GetNumberOfParameters() const;

  /** Get the list of parameters
   *
   * These are not the parameter values but instead a description of
   * the parameter. */
  virtual const std::vector< Parameter > & GetParameters() const;

  /** Get the number of active parameters. */
  unsigned int GetNumberOfActiveParameters() const;

  /** Check whether a parameter is active
   *
   * \param parameterName Name of the Parameter. */
  bool IsParameterActive( const std::string & parameterName );

  /** Sets a parameter value by name
   *
   * \param parameterName The name of the Parameter whose value should
   *  be set.
   * \param value The new value of the Parameter. */
  virtual ErrorType SetParameterValue( const std::string & parameterName,
                                       double value );

  /** Get the current value of a parameter
   *
   * \param parameterName The name of the Parameter whose value you
   *   want.
   * \return The value of the Parameter. */
  virtual double GetParameterValue( const std::string & parameterName );

  /** Compute the next set of parameters, output scalar values, and
   * log likelihood
   *
   * \return A new Sample. */
  virtual Sample NextSample() = 0;

  /** Get the current parameter values. */
  const std::vector< double > & GetCurrentParameters() const;

  /** Get the current log-likelihood. */
  double & GetCurrentLogLikelihood() const;

protected:
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

  /** Subclasses that need to reset internal state when a parameter
  value has been changed outside the operation of the optimization
  algorithm should override this method. */
  virtual void ParameterSetExternally() {};

  /**
     Which Parameters are active?  Indexes correspond to indexes
     within this->GetParameters().
  */
  std::vector< bool > m_ActiveParameterIndices;


}; // end Sampler

} // end namespace madai

#endif // madai_Sampler_h_included
