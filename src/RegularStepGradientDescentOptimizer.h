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

#ifndef __RegularStepGradientDescentOptimizer_h__
#define __RegularStepGradientDescentOptimizer_h__


#include "Optimizer.h"


namespace madai {

/** \class RegularStepGradientDescentOptimizer
 *
 * Straightforward implementation of a gradient descent optimizer.
 */
class RegularStepGradientDescentOptimizer : public Optimizer {
public:
  RegularStepGradientDescentOptimizer( const Model *model );
  ~RegularStepGradientDescentOptimizer();

  void NextIteration(Trace *trace);

  /** Set the step size. */
  void SetStepSize( double stepSize );

  /** Set search mode to minimize.
   *
   * Minimization is on by default. */
  void MinimizeOn();

  /** Set search mode to maximize. */
  void MinimizeOff();

protected:
  double m_StepSize;

  bool m_Minimize;

  RegularStepGradientDescentOptimizer() {}; // intentionally hidden
}; // end class RegularStepGradientDescentOptimizer

} // end namespace madai

#endif // __RegularStepGradientDescentOptimizer_h__
