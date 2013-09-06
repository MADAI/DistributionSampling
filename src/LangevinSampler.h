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

#ifndef madai_LangevinSampler_h_included
#define madai_LangevinSampler_h_included

#include "Sampler.h"


namespace madai {

/**
 * \class LangevinSampler
 *
 * This is an implementation of a Langevin search algorithm.
 * It considers the gradient of the likelihood at a point, and
 * moves according to the Langevin equation.
 *
 * \warning This class is experimental.
 */

class LangevinSampler : public Sampler {
public:
  // Constructor
  LangevinSampler();
  // Destructor
  virtual ~LangevinSampler();

  /**
   * Compute the next set of parameters, output scalar values, and
   * log likelihood
   *
   * \return A new Sample. */
  virtual Sample NextSample();
  
protected:
  /** Record of the largest gradient size. */
  double m_LargestGradient;
  
  /** Unweighted Average gradient with places chosen by using random gaussian steps. */
  double m_AverageGradient;
  
  /** Width used when taking a random gaussian step. */
  double m_GaussianWidth;
  
  /** Step size parameter. */
  double m_StepSize;
  
  /** Parameter keeping track of number of points used in calculating the average gradient. */
  unsigned int m_NumberOfElementsInAverage;

  /** Records the upper limits on the parameterspace based on the priors. */
  std::vector< double > m_UpperLimit;

  /** Records the lower limits on the parameterspace based on the priors. */
  std::vector< double > m_LowerLimit;

  /** Initialize this sampler with the given Model. */
  virtual void Initialize( const Model * model );
  
  /** Get the gradient of the LL at a point and update above parameters if necessary. */
  std::vector< double > GetGradient( const std::vector< double > Parameters, const Model * m );

private:

  /** Instance of a random number generator. */
  madai::Random m_Random;

}; // end class LangevinSampler

} // end namespace madai

#endif // madai_LangevinSampler_h_included
