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

#ifndef __Parameter_h__
#define __Parameter_h__

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
  /**
   * constructor.  default is a uniform prior on [0,1]
   */
  Parameter( std::string nm);

  /**
   * constructor.  default is a uniform prior on [min,max]
   */
  Parameter( std::string nm, double min, double max);

  /**
   * constructor.  Makes a copy of the supplied Distribution.
   */
  Parameter( std::string nm, const Distribution & distribution);

  //@{
  /**
   * copy constructors and assignmnet operator.
   */
  Parameter( const Parameter & other);
  Parameter & operator=( const Parameter & other);
  //@}

    /**
     * destructor.
     */
  virtual ~Parameter();

  /**
   * a short description of the parameter.
   */
  std::string m_Name;

  //@{
  /**
   * range of possible values for this Parameter
   * \warning depricated
   */
  double      m_MinimumPossibleValue;
  double      m_MaximumPossibleValue;
  //@}

  //@{
  /**
   * \todo document
   */
  const Distribution * GetPriorDistribution() const;
  Distribution * m_PriorDistribution;
  //@}

}; // end class Parameter

} // end namespace madai

#endif // __Parameter_h__
