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


bool
Trace
::Add( const Sample & element ) {
  if ( this->GetSize() > 0 ) {
    // Check for consistency with previous element in trace.
    const Sample & previousElement = m_Samples.back();
    if ( element.m_ParameterValues.size() != previousElement.m_ParameterValues.size() ) {
      return false;
    }
    if ( element.m_OutputValues.size() != previousElement.m_OutputValues.size() ) {
      return false;
    }
  }

  m_Samples.push_back( element );

  return true;
}


bool
Trace
::Add( const std::vector< double > & parameterValues,
       const std::vector< double > & outputValues,
       double logLikelihood )
{
  return this->Add( Sample( parameterValues, outputValues, logLikelihood ) );
}


bool
Trace
::Add( const std::vector< double > & parameterValues,
       const std::vector< double > & outputValues )
{
  return this->Add( Sample( parameterValues, outputValues ) );
}


bool
Trace
::Add( const std::vector< double > & parameterValues )
{
  return this->Add( Sample( parameterValues ) );
}


unsigned int
Trace
::GetSize() const
{
  return this->m_Samples.size();
}


void
Trace
::Clear()
{
  m_Samples.clear();
}


Sample &
Trace
::operator[]( unsigned int idx )
{
  return this->m_Samples[idx];
}


const Sample &
Trace
::operator[]( unsigned int idx ) const
{
  return this->m_Samples[idx];
}


/** Utility for writing a vector to an output stream
 *
 * The vector elements are delimited by the given delimiter */
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
    this->WriteCSVOutput( file, parameters, outputNames );
    file.close();
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


bool
Trace
::ImportCSVFile( const std::string & filename,
     int numberOfParameters,
     int numberOfOutputs )
{
  try {
    std::ifstream file( filename.c_str() );
    // Read in header
    for ( int i = 0; i < numberOfParameters; ++i ) {
      std::string parameterName;
      std::getline( file, parameterName, ',' );
      parameterName = parameterName.substr( 1, parameterName.size() - 2 );
    }

    for ( int i = 0; i < numberOfOutputs; ++i ) {
      std::string outputName;
      std::getline( file, outputName, ',' );
    }

    // Likelihood output
    std::string likelihoodName;
    std::getline( file, likelihoodName );
    likelihoodName = likelihoodName.substr( 1, likelihoodName.size() - 2 );

    // Now read data
    while ( !file.eof() ) {

      std::vector< double > parameterValues;
      for ( int i = 0; i < numberOfParameters; ++i ) {
  std::string parameterString;
  std::getline( file, parameterString, ',' );
  if ( file.eof() ) {
    break;
  }
  double parameterValue = atof( parameterString.c_str() );

  parameterValues.push_back( parameterValue );
      }

      std::vector< double > outputValues;
      for ( int i = 0; i < numberOfOutputs; ++i ) {
  std::string outputString;
  std::getline( file, outputString, ',' );
  if ( file.eof() ) {
    break;
  }
  double outputValue = atof( outputString.c_str() );
  outputValues.push_back( outputValue );
      }

      std::string logLikelihoodString;
      std::getline( file, logLikelihoodString );
      if ( file.eof() ) {
  break;
      }
      double logLikelihood = atof( logLikelihoodString.c_str() );

      this->Add( parameterValues, outputValues, logLikelihood );


    }

    file.close();
  } catch ( ... ) {
    // Reading failed
    return false;
  }

  // Reading succeeded
  return true;
}


/*
    Assert:
      FOR ALL i < this->m_Samples.size():
        this->m_Samples[i].m_ParameterValues.size() == params.size()
        this->m_Samples[i].m_OutputValues.size() == outputs.size()
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
    //std::cout << "Output name: " << *itr << std::endl;
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
    const Sample & sample = (*this)[i];
    write_vector( out, sample.m_ParameterValues, ',' );
    out << ',';

    if ( sample.m_OutputValues.size() > 0 ) {
      write_vector( out, sample.m_OutputValues, ',' );
      out << ',';
    }
    out << sample.m_LogLikelihood;
    if ( sample.m_Comments.size() > 0 ) {
      out << ",\"";
      write_vector( out, sample.m_Comments, ';' );
      out << '"';
    }
    out << '\n';
  }
  out.flush();
}


} // end namespace madai
