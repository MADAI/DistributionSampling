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


void
Trace
::Write( std::ostream & out ) const {
  unsigned int N = this->GetSize();
  for ( unsigned int i = 0; i < N; i++ ) {
    write_vector( out, (*this)[i].m_ParameterValues, ',' );
    out << ',';
    write_vector( out, (*this)[i].m_OutputValues, ',' );
    out << ',';
    out << (*this)[i].m_LogLikelihood;
    if ( (*this)[i].m_Comments.size() > 0 ) {
      out << ",\"";
      write_vector( out, (*this)[i].m_Comments, ';' );
      out << '"';
    }
    out << '\n';
  }
  out.flush();
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
::WriteHead( std::ostream & o,
             const std::vector< Parameter > & params ) const
{
  if ( !params.empty() ) {
    std::vector< Parameter >::const_iterator itr = params.begin();
    o << '"' << itr->m_Name << '"';
    for (itr++; itr < params.end(); itr++) {
      o << ',' << '"' << itr->m_Name << '"';
    }
  }
}


void
Trace
::PrintDataToFile( const std::vector< Parameter > & params )
{
#if 0
  std::cout << "Printing data to file." << std::endl;
  std::ofstream outputfile;
  std::stringstream ss;
  ss << m_WriteOutCounter+1;
  std::string out_file = m_TraceDirectory+"/output"+ss.str()+".dat";

  outputfile.open( out_file.c_str() );
  std::cout << "Writing out to: " << out_file << std::endl;
  if ( outputfile ) {
    outputfile << "#ITERATION,";
    if ( !params.empty() ) {
      std::vector< Parameter >::const_iterator itr = params.begin();
      for ( itr; itr < params.end(); itr++ ) {
        outputfile << itr->m_Name << ',';
      }
    }
    outputfile << std::endl;

    for ( int i = 0; i < m_Writeout; i++ ) {
      outputfile << i+m_WriteOutCounter*m_Writeout << ",";
      if ( !m_TraceElements[i].m_ParameterValues.empty() ) {
        for ( int j = 0; j < m_TraceElements[i].m_ParameterValues.size(); j++ ) {
          outputfile << m_TraceElements[i].m_ParameterValues[j];
          if ( j != m_TraceElements[i].m_ParameterValues.size()-1 ) {
            outputfile<< ",";
          }
        }
      }
    }
    outputfile << std::endl;
    outputfile.close();
  } else {
    std::cout << "Error: Couldn't open the output file" << std::endl;
    exit( 1 );
  }
#endif
}


void
Trace
::LoadTrace()
{
#if 0
  std::stringstream ss;
  ss << "cat ";

  for ( int i = 1; i <= ceil( (double) (m_MaxIterations) / (double)(m_Writeout) ); i++ ) {
    std::cout << "Parsing " << m_TraceDirectory << "/output" << i << ".dat" << std::endl;
    ss << m_TraceDirectory << "/output" << i << ".dat ";
  }
  ss << "> " << m_TraceDirectory << "/trace.dat" << std::endl;

  std::string command = ss.str();
  std::system( command.c_str() );

  ss.str( string() );
#endif
}

} // end namespace madai
