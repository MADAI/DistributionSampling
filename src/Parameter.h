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


namespace madai {


/**
 * \class Parameter
 * represents one input to a Model or ScalarFunction
 */
class Parameter {
public:
  /**
   * constructor.  Range defaults to [0,1]
   */
  Parameter( std::string nm, double mn = 0.0, double mx = 1.0 ) :
    m_Name(nm),
    m_MinimumPossibleValue(mn),
    m_MaximumPossibleValue(mx) { }

    /**
     * destructor.
     */
  virtual ~Parameter() { }

  /**
   * a short description of the parameter.
   */
  std::string m_Name;

  //@{
  /**
   * range of possible values for this Parameter
   */
  double      m_MinimumPossibleValue;
  double      m_MaximumPossibleValue;
  //@}
}; // end class Parameter

} // end namespace madai

#endif // __Parameter_h__
