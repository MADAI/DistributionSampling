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

#ifndef madai_PercentileGridSampler_h_included
#define madai_PercentileGridSampler_h_included


#include "Sampler.h"


namespace madai {

/**
 * \class PercentileGridSampler
 *
 * This Sampler generates an n-dimensional grid of Samples in the
 * joint percentile space of the parameters of the Model on which it
 * operates. Unlike other Samplers, it generates a fixed number of
 * Samples as determined by calling SetNumberOfSamples().
 */
class PercentileGridSampler : public Sampler {
public:
  PercentileGridSampler();
  virtual ~PercentileGridSampler();

  /** Get the next Sample from the distribution. */
  virtual Sample NextSample();

  //@{
  /**
   * Set/Get the number of samples desired. This number should equal
   *  n^p where p is the number of parameters in the Model and n is an
   *  integer.  If the pth root of the arugment is not an integer,
   *  then the pth root is rounded up to the nearest integer n and the
   *  number of samples will be n^p.
  */
  virtual void SetNumberOfSamples( unsigned int n );
  virtual unsigned int GetNumberOfSamples();
  //@}

  /**
     Reset the sampler to start at the first sample.
  */
  void Reset();

protected:
  /**
     Keeps track of which grid sample we are on. */
  std::vector<unsigned int> m_StateVector;

  /**
     Number of samples in each dimension. */
  unsigned int m_NumberOfSamplesInEachDimension;

protected:
  virtual void Initialize( const Model * model );

private:

}; // end class PercentileGridSampler

} // end namespace madai

#endif // madai_PercentileGridSampler_h_included
