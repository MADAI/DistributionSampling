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

#ifndef madai_MetropolisHastingsSampler_h_included
#define madai_MetropolisHastingsSampler_h_included


#include "Sampler.h"


namespace madai {

/**
 * \class MetropolisHastingsSampler
 *
 * This is an implementation of the Metropolis-Hastings
 * sampling algorithm.
 */
class MetropolisHastingsSampler : public Sampler {
public:
  MetropolisHastingsSampler();
  virtual ~MetropolisHastingsSampler();

  /** Get the next Sample from the distribution. */
  virtual Sample NextSample();

  //@{
  /** Set/Get the StepSize, which controls the average distance in Parameter
   *  space to move.
   *
   *  In each direction, the step distance is given by:
   *    step[i] = StepSize * StepScales[i] * RandomGaussian();
   *
   *  where StepScales[i] is precalulated as \f$p_{0.75} - p_{0.25}\f$
   *  and \f$p_{0.75}\f$ and \f$p_{0.25}\f$ are the third and first
   *  quartiles of the parameter prior distributions, respectively.
   */
  virtual void SetStepSize( double stepSize );
  virtual double GetStepSize() { return this->m_StepSize; }
  //@}

protected:
  /**
     Maximum distance in Parameter space to move, under euclidean L2
     norm.
  */
  double m_StepSize;

protected:
  virtual void Initialize( const Model * model );

  /** based on the length scales of the parameter space */
  std::vector< double > m_StepScales;

private:

  /** Random number object */
  madai::Random m_Random;
}; // end class MetropolisHastingsSampler

} // end namespace madai

#endif // madai_MetropolisHastingsSampler_h_included
