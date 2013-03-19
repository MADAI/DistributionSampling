/*=========================================================================
 *
 *  Copyright 2011-2013 The University of North Carolina at Chapel Hill
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

#ifndef madai_MetropolisHastingsSampler_h_included
#define madai_MetropolisHastingsSampler_h_included


#include "Sampler.h"


namespace madai {

/**
 * \class MetropolisHastingsSampler
 *
 * This is a simple implemetation of the Metropolis Hastings
 * algorithm.  It is "simple" in that each step is a point whose
 * direction is uniformly chosen from the unit sphere (L2-norm) around
 * the current point in parameter space.  The magnitude (L2-norm) of
 * the step is randomly chosen from the uniform distribution on the
 * range [0, StepSize].
 */
class MetropolisHastingsSampler : public Sampler {
public:
  /** Constructor. */
  MetropolisHastingsSampler();
  /** destructor */
  virtual ~MetropolisHastingsSampler();

  /** append the next point to the Trace  */
  virtual Sample NextSample();

  //@{
  /**
     Set/Get the StepSize, which controls the average distance in Parameter
     space to move.

     In each direction, the step distance is given by:
       step[i] = StepSize * StepScales[i] * RandomGaussian();

     Where StepScales[i] is precalulated as:
       = Model->GetParameters()[i].GetPriorDistribution()->GetPercentile(0.75)
       - Model->GetParameters()[i].GetPriorDistribution()->GetPercentile(0.25);
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
