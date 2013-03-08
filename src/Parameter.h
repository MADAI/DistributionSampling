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

#ifndef madai_Parameter_h_included
#define madai_Parameter_h_included

#include <string>
#include <vector>
#include "Distribution.h"


namespace madai {


/**
 * \class Parameter
 * represents one input to a Model or ScalarFunction
 */
class Parameter {
public:
  /** Constructor
   *
   * Default is a uniform prior on [0, 1] */
  Parameter( std::string nm);

  /** Constructor
   *
   * Default is a uniform prior on [min, max] */
  Parameter( std::string nm, double min, double max);

  /** Constructor
   *
   * Makes a copy of the supplied Distribution. */
  Parameter( std::string nm, const Distribution & distribution);

  //@{
  /** Copy constructors and assignment operator */
  Parameter( const Parameter & other);
  Parameter & operator=( const Parameter & other);
  //@}

  /** Destructor */
  virtual ~Parameter();

  /** A short description of the parameter */
  std::string m_Name;

  //@{
  /** Range of possible values for this Parameter
   *
   * \warning depricated
   */
  double      m_MinimumPossibleValue;
  double      m_MaximumPossibleValue;
  //@}

  //@{
  /** The distribution used as a prior for this parameter */
  const Distribution * GetPriorDistribution() const;
  Distribution * m_PriorDistribution;
  //@}

}; // end class Parameter

} // end namespace madai

#endif // madai_Parameter_h_included
