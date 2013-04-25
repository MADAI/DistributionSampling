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

#ifndef madai_GaussianDistribution_h_included
#define madai_GaussianDistribution_h_included

#include "Distribution.h"

#include "boost/math/distributions/normal.hpp"


namespace madai {

/** \class GaussianDistribution
 *
 * Provides access to various aspects of a Gaussian (or normal) distribution. */
class GaussianDistribution : public Distribution {
public:
  /** Constructor */
  GaussianDistribution();

  /** Destructor */
  virtual ~GaussianDistribution();

  virtual Distribution * Clone() const;

  /** Set the mean of the Gaussian distribution. */
  void SetMean( double mean );

  /** Get the mean of the Gaussian distribution. */
  double GetMean() const;

  /** Set the standard deviation of the Gaussian distribution. */
  void SetStandardDeviation( double standardDeviation );

  /** Get the standard deviation of the Gaussian distribution. */
  double GetStandardDeviation() const;

  virtual double GetLogProbabilityDensity( double value ) const;
  virtual double GetProbabilityDensity( double value ) const;
  virtual double GetPercentile( double percentile ) const;
  virtual double GetSample(madai::Random & r) const;

protected:
  /** Mean of the distribution. */
  double m_Mean;

  /** Standard deviation of the distribution. */
  double m_StandardDeviation;

  /** Compute the normalization. */
  inline double GetNormalizationFactor() const;

  /** Compute the exponent. */
  inline double GetExponent( double value ) const;

private:
  /** Underlying class from Boost library that computes the things we want. */
  boost::math::normal * m_InternalDistribution;

};

} // end namespace madai

#endif // madai_GaussianDistribution_h_included
