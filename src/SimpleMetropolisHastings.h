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

#ifndef __SimpleMetropolisHastings_h__
#define __SimpleMetropolisHastings_h__


#include "Optimizer.h"


namespace madai {

class SimpleMetropolisHastings : public Optimizer {
public:
  SimpleMetropolisHastings( const Model *model );
  ~SimpleMetropolisHastings();

  void NextIteration(Trace *trace);

  /** Set the step size. */
  void SetStepSize( double stepSize );

protected:
  double m_StepSize;

  SimpleMetropolisHastings() {}; // intentionally hidden

  std::vector< bool > m_ActiveParameters;

  unsigned int m_NumberOfParameters;
  
  unsigned int m_NumberOfOutputs;

}; // end class SimpleMetropolisHastings

} // end namespace madai

#endif // __SimpleMetropolisHastings_h__
