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
  virtual ~SimpleMetropolisHastingsSampler();

  virtual void NextSample(Trace *trace);

  /** Set the step size. */
  virtual void SetStepSize( double stepSize );

protected:
  double m_StepSize;

  SimpleMetropolisHastingsSampler() {}; // intentionally hidden

  unsigned int m_NumberOfParameters;

  unsigned int m_NumberOfOutputs;

  std::vector< double > m_lastStep_x;

  std::vector< double > m_lastStep_y;

  double m_lastStep_logLikelihood;

}; // end class SimpleMetropolisHastingsSampler

} // end namespace madai

#endif // __SimpleMetropolisHastingsSampler_h__
