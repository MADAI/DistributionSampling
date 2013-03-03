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

#include "TraceElement.h"


namespace madai {


TraceElement
::TraceElement( const std::vector< double > & parameter_values,
                const std::vector< double > & output_values,
                double LogLikelihood ) :
  m_ParameterValues( parameter_values ),
  m_OutputValues( output_values ),
  m_LogLikelihood( LogLikelihood )
{
}


TraceElement
::TraceElement( const std::vector< double > & parameter_values,
                const std::vector< double > & output_values ) :
  m_ParameterValues( parameter_values ),
  m_OutputValues( output_values ),
  m_LogLikelihood( 0.0 )
{
}


TraceElement
::TraceElement( const std::vector< double > & parameter_values) :
  m_ParameterValues( parameter_values ),
  m_LogLikelihood( 0.0 )
{
}


void
TraceElement
::Reset()
{
  m_ParameterValues.clear();
  m_OutputValues.clear();
  m_Comments.clear();
  m_LogLikelihood= 0.0;
}


bool
TraceElement
::IsValid() const {
  return ( m_ParameterValues.size() > 0
	   || m_OutputValues.size() > 0
	   || m_Comments.size() > 0 );
}


TraceElement
::TraceElement() :
  m_LogLikelihood(0.0)
{
}

} // end namespace madai
