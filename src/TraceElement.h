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

#ifndef __TraceElement_h__
#define __TraceElement_h__

#include <string>
#include <vector>


namespace madai {


/** \class TraceElement
 *
 * Individual entry in a Trace.
 */
class TraceElement {
public:
  TraceElement(const std::vector< double > & parameterValues,
               const std::vector< double > & OutputValues,
               double LogLikelihood);

  TraceElement(const std::vector< double > & parameterValues,
               const std::vector< double > & OutputValues );

  TraceElement(const std::vector< double > & parameterValues);

  TraceElement();
  void Reset();
  void Print();
  void VizTrace();

  std::vector< double > m_ParameterValues;
  std::vector< double > m_OutputValues;
  double                m_LogLikelihood;
  bool                  m_InTrace;

  /** Comments may be used to store human-readable comments *or*
  record changes to state, such as changing an optimizer type,
  which parameters are activated, etc.. */
  std::vector< std::string > m_Comments;

}; // class TraceElement

} // end namespace madai

#endif // __TraceElement_h__
