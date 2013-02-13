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

class Parameter {
public:
  Parameter( std::string nm, double mn = 0.0, double mx = 1.0 ) :
    m_Name(nm),
    m_MinimumPossibleValue(mn),
    m_MaximumPossibleValue(mx) { }
  virtual ~Parameter() { }

  std::string m_Name;
  double      m_MinimumPossibleValue;
  double      m_MaximumPossibleValue;
}; // end class Parameter

} // end namespace madai

#endif // __Parameter_h__
