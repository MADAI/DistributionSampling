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

class ScalarFunction {
 public:
  typedef enum {
    NO_ERROR = 0,
    INVALID_PARAMETER_INDEX,
    METHOD_NOT_IMPLEMENTED,
    OTHER_ERROR
  } ErrorType;

  ScalarFunction();
  virtual ~ScalarFunction();

  /** Get the number of parameters. */
  virtual unsigned int GetNumberOfParameters() const = 0;
  /** Get names of the parameters. */
  virtual const std::vector< Parameter > & GetParameters() const = 0;
  /** Get the valid range for the parameter at parameterIndex. */
  virtual ErrorType GetRange(
    unsigned int parameterIndex, double range[2] ) const = 0;

  /** Get the name of the scalar output. */
  virtual const std::string & GetScalarOutputName() const = 0;

  /** evaluate the function at this point. */
  virtual ErrorType GetOutput( const std::vector< double > & parameters,
                               double  &  output) = 0;
};
} // end namespace madai
#endif /* __ScalarFunction_h__ */
