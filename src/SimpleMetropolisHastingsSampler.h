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

/**
 * \class SimpleMetropolisHastingsSampler
 *
 * This is a simple implemetation of the Metropolis Hastings
 * algorithm.  It is "simple" in that each step is a point whose
 * direction is uniformly chosen from the unit sphere (L2-norm) around
 * the current point in parameter space.  The magnitude (L2-norm) of
 * the step is randomly chosen from the uniform distribution on the
 * range [0, StepSize].
 */
class SimpleMetropolisHastingsSampler : public Sampler {
public:
  /** constructor.  No way right now to change the model mid-trace.
      Only parameter values can change  */
  SimpleMetropolisHastingsSampler( const Model *model );
  /** destructor */
  virtual ~SimpleMetropolisHastingsSampler();

  /** append the next point to the Trace  */
  virtual Sample NextSample();

  //@{
  /**
     Set/Get the StepSize, which is the maximum distance in Parameter
     space to move, under euclidean L2 norm.
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

  /** cache this value from the Model */
  unsigned int m_NumberOfParameters;

  /** cache this value from the Model */
  unsigned int m_NumberOfOutputs;

protected:
  /** based on the length scales of the parameter space */
  std::vector< double > m_stepScales;

private:
  /** This Sampler needs a Model to sample from, so this constructor
      is hidden. */
  SimpleMetropolisHastingsSampler() {};

  /**
   * random number object.
   */
  madai::Random m_random;
}; // end class SimpleMetropolisHastingsSampler

} // end namespace madai

#endif // __SimpleMetropolisHastingsSampler_h__
