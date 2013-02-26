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

#ifndef __ScalarFunction_h__
#define __ScalarFunction_h__

#include "Parameter.h"
#include <vector>

namespace madai {

/**
 * Class ScalarFunction.
 *
 * Represents a single scalar evaluated at a point in parameter space.
 *
 * Meant to be used to represent prior-log-likelihood.
 */
class ScalarFunction {
public:
  typedef enum {
    NO_ERROR = 0,
    INVALID_PARAMETER_INDEX,
    METHOD_NOT_IMPLEMENTED,
    OTHER_ERROR
  } ErrorType;

	ScalarFunction() : m_ScalarOutputName("output") { }
  virtual ~ScalarFunction();

  /** Get the number of parameters. */
  virtual unsigned int GetNumberOfParameters() const
	{
		return this->m_Parameters.size();
	}

  /** Get names of the parameters. */
  virtual const std::vector< Parameter > & GetParameters() const
	{
		return this->m_Parameters;
	}

  /** Get the valid range for the parameter at parameterIndex. */
  virtual ErrorType GetRange(unsigned int parameterIndex, double range[2] ) {
		if (parameterIndex >= this->m_Parameters.size())
			return INVALID_PARAMETER_INDEX;
		range[0] = this->m_Parameters[parameterIndex].m_MinimumPossibleValue;
		range[0] = this->m_Parameters[parameterIndex].m_MaximumPossibleValue;
		return NO_ERROR;
	}

  /** Get the name of the scalar output. */
  virtual const std::string & GetScalarOutputName() const {
		return this->m_ScalarOutputName; // default name
	};

  /** evaluate the function at this point. */
  virtual ErrorType GetOutput( const std::vector< double > & parameters,
                               double & output) = 0; // must be overridden
protected:
  std::vector< Parameter > m_Parameters;
	std::string m_ScalarOutputName;
};
} // end namespace madai
#endif /* __ScalarFunction_h__ */
