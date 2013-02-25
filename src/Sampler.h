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
 * Base class for algorithms that sample from a distribution. */
class Sampler {
public:
  typedef enum {
    NO_ERROR = 0,
    INVALID_PARAMETER_INDEX_ERROR,
    INVALID_OUTPUT_SCALAR_INDEX_ERROR
  } ErrorType;

  Sampler( const Model *model );
  virtual ~Sampler();
  const Model * GetModel() const;

  /**
   * deprecated
   */
  std::set< std::string > GetActiveParameters();

  /**
   * deprecated
   */
  void ActivateParameter( const std::string & parameterName );
  /**
   * deprecated
   */
  void DeactivateParameter( const std::string & parameterName );

  /**
   * easiest way to activate a parameter
   */
  void ActivateParameter( unsigned int ParameterIndex );
  /**
   * easiest way to deactivate a parameter
   */
  void DeactivateParameter( unsigned int ParameterIndex );

  virtual unsigned int GetNumberOfParameters() const;

  virtual const std::vector< Parameter > & GetParameters() const;

  /** Get the number of active parameters. */
  unsigned int GetNumberOfActiveParameters() const;

  /** Resets a parameter value. */
  virtual ErrorType SetParameterValue( const std::string & parameterName,
                                       double value );

  /** Sets the output scalar value to optimize. */
  ErrorType SetOutputScalarToOptimize( const std::string & scalarName );
  std::string GetOutputScalarToOptimize();

  ErrorType SetOutputScalarToOptimizeIndex(unsigned int idx);
  unsigned int GetOutputScalarToOptimizeIndex() const;

  /** Compute the next set of parameters and the output scalar values,
   * and save them in the trace file. */

  virtual void NextSample(Trace *trace) = 0;
  //{  /* suggested structure for this function */
  //std::vector< double > scalarOutputs;
  //std::vector< double > gradient;
  //int err;
  //err = m_Model->GetScalarAndGradientOutputs(
  // m_CurrentParameters,
  // m_ActiveParameters,
  // scalarOutputs,
  // m_OutputScalarToOptimizeIndex,
  // gradient);
  //    if (err) {
  //      // handle the error
  //    }
  //m_Trace->RecordData(m_CurrentParameters, scalarOutputs);
  //
  // // Based on:
  // //    scalarOutputs[m_OutputScalarToOptimizeIndex]
  // //    m_ActiveParameters
  // //    m_Trace
  // //    gradient
  // //    m_CurrentParameters
  // // Then we need to update
  // //    m_CurrentParameters
  //}

  // scalars;
  // m_Model->GetScalarOutputs( currentPosition, scalars, gradient );
  // m_Trace->RecordData( scalars )

  // update position

  /** Get the current parameter values. */
  const std::vector< double > & GetCurrentParameters() const;


  /**
   * If true, make a distribution proportional to exp(LogPosteriorLikelihood)
   * If false, make a distribution proportional to m_OutputScalarToOptimize
   *
   * LogPosteriorLikelihood comes from the
   * m_Model->GetScalarOutputsAndLogLikelihood() function;
   */
  virtual bool GetOptimizeOnLikelihood() const;
  /**
   * If true, make a distribution proportional to exp(LogPosteriorLikelihood)
   * If false, make a distribution proportional to m_OutputScalarToOptimize
   *
   * LogPosteriorLikelihood comes from the
   * m_Model->GetScalarOutputsAndLogLikelihood() function;
   */
  virtual void SetOptimizeOnLikelihood(bool val);

protected:
  Sampler() {}; // intentionally hidden
  unsigned int GetOutputScalarIndex( const std::string & scalarName ) const;
  unsigned int GetParameterIndex( const std::string & parameterName ) const;


  /**
   * WHAT IS THIS?
   */
  bool IsLikeAndPrior() const;

  const Model *           m_Model;
  std::set< std::string > m_ActiveParameters;
  std::vector< double >   m_CurrentParameters;
  std::string             m_OutputScalarToOptimize;
  unsigned int            m_OutputScalarToOptimizeIndex;

  /**
   * If true, make a distribution proportional to exp(LogPosteriorLikelihood)
   * If false, make a distribution proportional to m_OutputScalarToOptimize
   *
   * LogPosteriorLikelihood comes from the
   * m_Model->GetScalarOutputsAndLogLikelihood() function;
   */
  bool                    m_OptimizeOnLikelihood;


  /** Subclasses that need to reset internal state when a parameter
  value has been changed outside the operation of the optimization
  algorithm should override this method. */
  virtual void ParameterSetExternally() {};

  std::vector< bool > m_ActiveParameterIndices;


}; // end Sampler

} // end namespace madai

#endif // __Sampler_h__x
