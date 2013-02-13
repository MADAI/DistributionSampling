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

#include <gsl/gsl_randist.h>

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
    OTHER_ERROR
  } ErrorType;

  Model();
  virtual ~Model();

  /** Loads a configuration from a file. **/
  virtual ErrorType LoadConfigurationFile( const std::string fileName ) = 0;

  /** Has the model been initialized? */
  bool IsReady() const;

  /** Get the number of parameters. */
  virtual unsigned int GetNumberOfParameters() const;

  /** Get names of the parameters. */
  virtual const std::vector< Parameter > & GetParameters() const;

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

}; // end Model

} // end namespace madai

#endif // __Model_h
