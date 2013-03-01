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

#include "Trace.h"

namespace madai {


Trace
::Trace()
{
  // Nothing to initialize
}


Trace
::~Trace()
{
}


void
Trace
::Add( const std::vector< double > & parameterValues,
       const std::vector< double > & outputValues,
       double logLikelihood )
{
  m_TraceElements.push_back( TraceElement( parameterValues,
                                           outputValues,
                                           logLikelihood) );
}


void
Trace
::Add( const std::vector< double > & parameterValues,
       const std::vector< double > & outputValues )
{
  m_TraceElements.push_back( TraceElement( parameterValues,
                                           outputValues ) );
}


void
Trace
::Add( const std::vector< double > & parameterValues )
{
  m_TraceElements.push_back( TraceElement( parameterValues ) );
}


unsigned int
Trace
::GetSize() const
{
  return this->m_TraceElements.size();
}


TraceElement &
Trace
::operator[]( unsigned int idx )
{
  return this->m_TraceElements[idx];
}


const TraceElement &
Trace
::operator[]( unsigned int idx ) const
{
  return this->m_TraceElements[idx];
}


template <class T>
void write_vector( std::ostream& o, std::vector< T > const & v, char delim ) {
  if ( !v.empty() ) {
    typename std::vector< T >::const_iterator itr = v.begin();
    o << *(itr++);
    while ( itr < v.end() ) {
      o << delim << *(itr++);
    }
  }
}


bool
Trace
::WriteCSVFile( const std::string & filename,
		const std::vector< Parameter > & parameters,
		const std::vector< std::string > & outputNames ) const
{
  try {
    std::ofstream file( filename.c_str() );
    this->WriteCSVOutput( file, parameters );
  } catch ( ... ) {
    return false;
  }

  return true;
}


void
Trace
::WriteCSVOutput( std::ostream & os,
                  const std::vector< Parameter > & parameters,
                  const std::vector< std::string > & outputNames ) const
{
  this->WriteHead( os, parameters, outputNames );
  this->WriteData( os );
}


/*
    Assert:
      FOR ALL i < this->m_TraceElements.size():
        this->m_TraceElements[i].m_ParameterValues.size() == params.size()
        this->m_TraceElements[i].m_OutputValues.size() == outputs.size()
  */
void
Trace
::WriteHead( std::ostream & o,
             const std::vector< Parameter > & params,
             const std::vector< std::string > & outputs) const
{
  if ( !params.empty() ) {
    std::vector< Parameter >::const_iterator itr = params.begin();
    o << '"' << itr->m_Name << '"';
    for ( itr++; itr < params.end(); itr++ ) {
      o << ',' << '"' << itr->m_Name << '"';
    }
    if ( !outputs.empty() ) {
      o << ',';
    }
  }
  if ( !outputs.empty() ) {
    std::vector<std::string>::const_iterator itr = outputs.begin();
    o << '"' << *itr << '"';
    for ( itr++; itr < outputs.end(); itr++ ) {
      o << ',' << '"' << *itr << '"';
    }
  }
  o << ",\"LogLikelihood\"\n";
}


void
Trace
::WriteData( std::ostream & out ) const {
  unsigned int n = this->GetSize();
  for ( unsigned int i = 0; i < n; i++ ) {
    const TraceElement & element = (*this)[i];
    write_vector( out, element.m_ParameterValues, ',' );
    out << ',';

    if ( element.m_OutputValues.size() > 0 ) {
      write_vector( out, element.m_OutputValues, ',' );
      out << ',';
    }
    out << element.m_LogLikelihood;
    if ( element.m_Comments.size() > 0 ) {
      out << ",\"";
      write_vector( out, element.m_Comments, ';' );
      out << '"';
    }
    out << '\n';
  }
  out.flush();
}



} // end namespace madai
