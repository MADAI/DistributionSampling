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
  virtual double GetStandardDeviation() const;

  virtual double GetLogProbabilityDensity( double x ) const;
  virtual double GetGradientLogProbabilityDensity( double x ) const;
  virtual double GetProbabilityDensity( double x ) const;
  virtual double GetPercentile( double percentile ) const;
  virtual double GetSample(madai::Random & r) const;

  /**
     Returns E[x] */
  virtual double GetExpectedValue() const;

protected:
  /** Mean of the distribution. */
  double m_Mean;

  /** Standard deviation of the distribution. */
  double m_StandardDeviation;

  /** Get the normalization factor (the portion of the Gaussian
   *  function that multiplies the exponential). */
  inline double GetNormalizationFactor() const;

  /** Get the exponent passed to the exponential function.
   *
   * \param x The argument to the probability density function. */
  inline double GetExponent( double x ) const;

private:
  /** Explicitly disallowed */
  GaussianDistribution& operator=(const madai::GaussianDistribution &);

  /** Explicitly disallowed */
  GaussianDistribution(const madai::GaussianDistribution &);

  struct GaussianDistributionPrivate;

  /** Opaque Pointer for Private Implementation */
  GaussianDistributionPrivate * m_GaussianDistributionImplementation;

};

} // end namespace madai

#endif // madai_GaussianDistribution_h_included
