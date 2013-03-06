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

#ifndef __Sampler_h__
#define __Sampler_h__

#include <set>

#include "Model.h"
#include "Trace.h"
#include "Parameter.h"

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

  /** default constructor. */
  Sampler( const Model *model );
  /** default constructor. */
  virtual ~Sampler();
  /** return a pointer to the current Model */
  const Model * GetModel() const;

  /** git a list of the names of the active parameters. */
  std::set< std::string > GetActiveParameters();

  /** Activate a parameter by name. */
  void ActivateParameter( const std::string & parameterName );

  /**
   * Activate a parameter by index.  Indexes correspond to indexes
   * within this->GetParameters().
   */
  void ActivateParameter( unsigned int parameterIndex );

  /** Deactivate a parameter by name. */
  void DeactivateParameter( const std::string & parameterName );

  /**
   * Deactivate a parameter by index. Indexes correspond to indexes
   * within this->GetParameters().
   */
  void DeactivateParameter( unsigned int parameterIndex );

  /** Get the number of parameters. */
  virtual unsigned int GetNumberOfParameters() const;

  /** Get the list of parameters. These are not the parameter values
   * but instead a description of the parameter. */
  virtual const std::vector< Parameter > & GetParameters() const;

  /** Get the number of active parameters. */
  unsigned int GetNumberOfActiveParameters() const;

  /** Check whether a parameter is active. */
  bool IsParameterActive( const std::string & parameterName );

  /** Resets a parameter value. */
  virtual ErrorType SetParameterValue( const std::string & parameterName,
                                       double value );
  /** Get the current value of a parameter. */
  virtual double GetParameterValue( const std::string & parameterName );

  /** Compute the next set of parameters and the output scalar values,
   * and save them in the trace file. */
  virtual void NextSample(Trace *trace) = 0;

  /** Get the current parameter values. */
  const std::vector< double > & GetCurrentParameters() const;
  double & GetCurrentLogLikelihood() const;

protected:
  /** intentionally hidden */
  Sampler() {};

  /** useful for a implementation */
  unsigned int GetParameterIndex( const std::string & parameterName ) const;

  /** The Model from which this Sampler takes samples. */
  const Model *           m_Model;

  /** Set of strings naming the active parameters. */
  std::set< std::string > m_ActiveParameters;

  /** Stores the point in parameter space. This will change when
   *  NextSample() is called. */
  std::vector< double >   m_CurrentParameters;
  double                  m_CurrentLogLikelihood;

  /**
   *
   * LogPosteriorLikelihood comes from the
   * m_Model->GetScalarOutputsAndLogLikelihood() function;
   */
  bool                    m_OptimizeOnLikelihood;

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

#endif // __Sampler_h__x
