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

#ifndef __UniformDistribution_h__
#define __UniformDistribution_h__

#include "Distribution.h"


namespace madai {

class UniformDistribution : public Distribution {
public:
  UniformDistribution();
  virtual ~UniformDistribution();

  /** Set/get the minimum value of the uniform distribution. */
  void SetMinimum( double minimum );
  double GetMinimum() const;

  /** Set/get the maximum value of the uniform distribution. */
  void SetMaximum( double maximum );
  double GetMaximum() const;

  virtual double GetLogProbabilityDensity( double value ) const;
  virtual double GetProbabilityDensity( double value ) const;
  virtual double GetPercentile( double percentile ) const;
  virtual double GetSample(madai::Random & r) const;

protected:
  double m_Minimum;
  double m_Maximum;

  inline bool InRange( double value ) const;
};

} // end namespace madai

#endif // __UniformDistribution_h__
