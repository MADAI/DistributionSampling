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

#ifndef __SimpleMetropolisHastingsSampler_h__
#define __SimpleMetropolisHastingsSampler_h__


#include "Sampler.h"


namespace madai {

class SimpleMetropolisHastingsSampler : public Sampler {
public:
  SimpleMetropolisHastingsSampler( const Model *model );
  ~SimpleMetropolisHastingsSampler();

  void NextIteration(Trace *trace);

  /** Set the step size. */
  void SetStepSize( double stepSize );

protected:
  double m_StepSize;

  SimpleMetropolisHastingsSampler() {}; // intentionally hidden

  std::vector< bool > m_ActiveParameters;

  unsigned int m_NumberOfParameters;
  
  unsigned int m_NumberOfOutputs;

}; // end class SimpleMetropolisHastingsSampler

} // end namespace madai

#endif // __SimpleMetropolisHastingsSampler_h__
