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

#ifndef __Distribution_h__
#define __Distribution_h__

#include "Random.h"

namespace madai {

/** \class Distribution
 *
 * Base class for distributions. */
class Distribution {
public:
  Distribution();
  virtual ~Distribution();

  /** Return a copy of this object.
   *
   * The caller is responsible for deleting the object with the
   * operator 'delete'.
   * \return A copy of this object. */
  virtual Distribution * Clone() const = 0;

  /** Get the log of the probability density function evaluated at x.
   *
   * \param x The argument to the probability density function.
   * \return The log of the probability density function evaluated at
   * x. */
  virtual double GetLogProbabilityDensity(double x) const = 0;

  /** Get the probability density function evaluated at x.
   *
   * \param x The argument to the probability density function.
   * \return The result of evaluating the probability density function
   * at x. */
  virtual double GetProbabilityDensity(double x) const = 0;

  /** Get the percentile for the given argument.
   *
   * Returns the value at which the cumulative distribution function
   * equals the input argument.
   *
   * \param percentile The percentage of the distribution in the range
  [0, 1].
   * \return The percentile value. */
  virtual double GetPercentile(double percentile) const = 0;

  /** Get a random sample from this distribution
   *
   * \param r An instance of the Random generator class.
   * \return The random sample from the distribution. */
  virtual double GetSample(madai::Random & r) const = 0;

protected:

};

} // namespace madai

#endif
