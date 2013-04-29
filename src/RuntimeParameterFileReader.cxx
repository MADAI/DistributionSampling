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

#include "RuntimeParameterFileReader.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <vector>


namespace madai {

RuntimeParameterFileReader
::RuntimeParameterFileReader() :
  m_NumberOfArguments( 0 ),
  m_Arguments( NULL )
{
}

RuntimeParameterFileReader
::~RuntimeParameterFileReader()
{
  this->FreeMemory();
}

bool
RuntimeParameterFileReader
::ParseFile( const std::string fileName )
{
  this->FreeMemory();

  std::ifstream inFile( fileName.c_str() );
  if ( inFile.is_open() ) {
    std::string line;
    std::string element;
    std::vector< std::string > arg_list;
    m_NumberOfArguments = 0;
    while ( inFile.good() ) {
      std::getline( inFile, line );

      line = this->RegularizeLine( line );

      // Split the regularized string by the first space
      size_t firstSpace = line.find_first_of( ' ' );
      std::string name = line.substr( 0, firstSpace );
      if ( name.size() > 0 ) {
        if ( firstSpace != std::string::npos ) {
          std::string value = line.substr( firstSpace+1 );
          m_Options[ name ] = value;
        } else {
          m_Options[ name ] = std::string();
        }
        arg_list.push_back( name );

        // Push rest of tokens onto the arg_list
        while ( firstSpace != std::string::npos ) {
          size_t nextSpace = line.find_first_of( ' ', firstSpace+1 );
          std::string token = line.substr( firstSpace+1,
                                           (nextSpace - firstSpace - 1) );
          arg_list.push_back( token );
          firstSpace = nextSpace;
        }
      }
    }

    m_NumberOfArguments = arg_list.size();
    m_Arguments = new char*[m_NumberOfArguments]();
    for ( int i = 0; i < m_NumberOfArguments; i++ ) {
      m_Arguments[i] = new char[arg_list[i].size()+1];
      std::strcpy( m_Arguments[i], arg_list[i].c_str() );
    }
  } else {
    std::cerr << "RuntimeParameterFileReader couldn't find input file'" << fileName << "'\n";
    return false;
  }

  return true;
}

bool
RuntimeParameterFileReader
::HasOption(const std::string & key) const
{
  return (m_Options.count(key) > 0);
}

const std::string &
RuntimeParameterFileReader
::GetOption(const std::string & key) const
{
  static const std::string empty("");
  if (this->HasOption(key)) {
    //return m_Options[key]; doesn't work because operator [] isn't const
    return m_Options.find( key )->second;
  } else {
    return empty;
  }
}

double
RuntimeParameterFileReader
::GetOptionAsDouble(const std::string & key) const
{
  return std::atof(this->GetOption(key).c_str());
}

long
RuntimeParameterFileReader
::GetOptionAsInt(const std::string & key) const
{
  return std::atol(this->GetOption(key).c_str());
}

void
RuntimeParameterFileReader
::PrintAllOptions(std::ostream & out) const
{
  for (std::map<std::string, std::string>::const_iterator it =
         m_Options.begin(); it != m_Options.end(); ++it)
    out << "Options[\"" << it->first << "\"] = \"" << it->second << "\"\n";
}

const std::map<std::string, std::string>
RuntimeParameterFileReader
::GetAllOptions() const
{
  return m_Options; // implicit cast to const
}


int
RuntimeParameterFileReader
::GetNumberOfArguments() const
{
  return m_NumberOfArguments;
}

char **
RuntimeParameterFileReader
::GetArguments() const
{
  return m_Arguments;
}

void
RuntimeParameterFileReader
::FreeMemory()
{
  for ( int i = 0; i < m_NumberOfArguments; i++ ) {
    delete[] m_Arguments[i];
  }
  delete[] m_Arguments;

  m_Arguments = NULL;
}

bool IsSameWhitespace( char c1, char c2 )
{
  return ( ::isspace( c1 ) && ::isspace( c2 ) );
}

char TabToSpace( char input )
{
  if ( input == '\t' ) {
    return ' ';
  }

  return input;
}

std::string
RuntimeParameterFileReader
::RegularizeLine( std::string line )
{
  // Chop off anything after comment character
  size_t commentPosition = line.find_first_of( '#' );
  line = line.substr( 0, commentPosition );

  // Convert any tabs to spaces
  std::transform( line.begin(), line.end(), line.begin(), TabToSpace );

  // Convert contiguous white space in line to a single space
  std::string::iterator newEnd = std::unique( line.begin(), line.end(), IsSameWhitespace );
  line.resize( std::distance( line.begin(), newEnd ) );

  // Trim whitespace left
  if ( *(line.begin()) == ' ' ) {
    line.erase( line.begin() );
  }

  // Trim whitespace right
  if ( *(line.end() - 1) == ' ' ) {
    line.erase( (line.end() - 1) );
  }

  return line;
}

} // end namespace madai
