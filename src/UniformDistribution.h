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

#ifndef madai_UniformDistribution_h_included
#define madai_UniformDistribution_h_included

#include "Distribution.h"


namespace madai {

/** \class UniformDistribution
 *
 * Provides access to various aspects of a uniform distribution. */
class UniformDistribution : public Distribution {
public:
  UniformDistribution();
  virtual ~UniformDistribution();

  virtual Distribution * Clone() const;

  /** Set the minimum value of the uniform distribution. */
  void SetMinimum( double minimum );

  /** Get the minimum value of the uniform distribution. */
  double GetMinimum() const;

  /** Set the maximum value of the uniform distribution. */
  void SetMaximum( double maximum );

  /** Get the maximum value of the uniform distribution. */
  double GetMaximum() const;


  virtual double GetLogProbabilityDensity( double x ) const;

  virtual double GetProbabilityDensity( double x ) const;

  virtual double GetPercentile( double percentile ) const;

  virtual double GetSample(madai::Random & r) const;

protected:
  /** Minimum value at which the probability density is non-zero. */
  double m_Minimum;

  /** Maximum value at which the probability density is non-zero. */
  double m_Maximum;

  /** Indicates whether the given value is in the range [m_Minimum,
   *  m_Maximum]
   *
   * \param x The argument to the probability density function.
   * \return True if the parameter value is in the range, false otherwise. */
  inline bool InRange( double x ) const;
};

} // end namespace madai

#endif // madai_UniformDistribution_h_included
