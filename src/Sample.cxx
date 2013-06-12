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

#include <ostream>

#include "Sample.h"


namespace madai {


Sample
::Sample( const std::vector< double > & parameter_values,
    const std::vector< double > & output_values,
    double LogLikelihood ) :
  m_ParameterValues( parameter_values ),
  m_OutputValues( output_values ),
  m_LogLikelihood( LogLikelihood )
{
}


Sample
::Sample( const std::vector< double > & parameter_values,
    const std::vector< double > & output_values ) :
  m_ParameterValues( parameter_values ),
  m_OutputValues( output_values ),
  m_LogLikelihood( 0.0 )
{
}


Sample
::Sample( const std::vector< double > & parameter_values ) :
  m_ParameterValues( parameter_values ),
  m_LogLikelihood( 0.0 )
{
}


void
Sample
::Reset()
{
  m_ParameterValues.clear();
  m_OutputValues.clear();
  m_Comments.clear();
  m_LogLikelihood= 0.0;
}


bool
Sample
::IsValid() const {
  return ( m_ParameterValues.size() > 0
     || m_OutputValues.size() > 0
     || m_Comments.size() > 0 );
}


Sample
::Sample() :
  m_LogLikelihood(0.0)
{
}

/**
   Provide this operator so that we can do:

     void SortSamples(std::vector< Sample > & s) {
       std::sort(s.begin(),s.end());
     }
*/
bool Sample
::operator<(const Sample & rhs) const {
    return (m_LogLikelihood < rhs.m_LogLikelihood);
}


std::ostream & operator<<(std::ostream & os, const Sample & sample) {

  os << "Sample:\n";
  os << "  ParameterValues: [";
  for ( size_t i = 0; i < sample.m_ParameterValues.size(); ++i ) {
    os << sample.m_ParameterValues[i];
    if ( i < sample.m_ParameterValues.size()-1) os << ", ";
  }
  os << "]\n";

  os << "  OutputValues: [";
  for ( size_t i = 0; i < sample.m_OutputValues.size(); ++i ) {
    os << sample.m_OutputValues[i];
    if ( i < sample.m_OutputValues.size()-1) os << ", ";
  }
  os << "]\n";

  os << "  LogLikelihood: [" << sample.m_LogLikelihood << "]\n";

  if ( sample.m_Comments.size() > 0 ) {
    for ( size_t i = 0; i < sample.m_Comments.size(); ++i ) {
      os << sample.m_Comments[i];
      if ( i < sample.m_Comments.size()-1) os << ", ";
    }
  }

  return os;
}

} // end namespace madai
