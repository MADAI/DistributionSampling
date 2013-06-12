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
 * This sampler will return ~N samples from the *prior* distribution.
 */
class PercentileGridSampler : public Sampler {
public:
  /** Constructor. */
  PercentileGridSampler();
  /** destructor */
  virtual ~PercentileGridSampler();

  /** append the next point to the Trace  */
  virtual Sample NextSample();

  //@{
  /**
     Set/Get the N, which controls the number of samples.  N should
     equal n^p for p=model->NumberOfParameters and n is an integer.
     if N it not, it will be rounded up.
  */
  virtual void SetNumberOfSamples( unsigned int N );
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
