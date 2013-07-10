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

#ifndef madai_Parameter_h_included
#define madai_Parameter_h_included

#include <string>
#include <vector>
#include "Distribution.h"


namespace madai {


/**
 * \class Parameter
 *
 * Represents one input to a Model. A Parameter has a name and a prior
 * distribution.
 */
class Parameter {
public:
  /** Constructor
   *
   * Defaults to a uniform prior distribution with range [0, 1]. */
  Parameter( std::string nm);

  /** Constructor
   *
   * Unuseable state */
  Parameter( );

  /** Constructor
   *
   * Default is a uniform prior on [min, max] */
  Parameter( std::string nm, double min, double max);

  /** Constructor
   *
   * Makes a copy of the supplied Distribution so the one passed in as
   * an arugment may be disposed of after this constructor is called. */
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
  /** The distribution used as a prior for this parameter */
  const Distribution * GetPriorDistribution() const;
  Distribution * m_PriorDistribution;
  //@}

}; // end class Parameter

} // end namespace madai

#endif // madai_Parameter_h_included
