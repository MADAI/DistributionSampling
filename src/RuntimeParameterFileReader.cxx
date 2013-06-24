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

#include <boost/algorithm/string.hpp>


namespace madai {

RuntimeParameterFileReader
::RuntimeParameterFileReader()
{
}

RuntimeParameterFileReader
::~RuntimeParameterFileReader()
{
}

bool
RuntimeParameterFileReader
::ParseFile( const std::string fileName )
{
  std::ifstream inFile( fileName.c_str() );
  if ( inFile.good() ) {
    std::string line;
    std::string element;
    while ( inFile.good() ) {
      std::getline( inFile, line );

      // This gets rid of leading and trailing whitespace and chops
      // off comments
      line = this->RegularizeLine( line );

      // Split the regularized string by the first space
      size_t firstSpace = line.find_first_of( ' ' );
      std::string name = line.substr( 0, firstSpace );
      if ( name.size() > 0 ) {
        if ( firstSpace != std::string::npos ) {
          std::string value( line.substr( firstSpace+1 ) );
          boost::algorithm::trim( value );
          m_Options[ name ] = value;
        } else {
          m_Options[ name ] = std::string();
        }
      }
    }
  } else {
    std::cerr << "RuntimeParameterFileReader couldn't find input file'"
      << fileName << "'\n";
    return false;
  }

  return true;
}

bool
RuntimeParameterFileReader
::HasOption(const std::string & key) const
{
  return ( m_Options.count( key ) > 0 );
}

const std::string &
RuntimeParameterFileReader
::GetOption(const std::string & key) const
{
  static const std::string empty("");
  if (this->HasOption( key )) {
    //return m_Options[key]; doesn't work because operator [] isn't const
    return m_Options.find( key )->second;
  } else {
    return empty;
  }
}

const std::string &
RuntimeParameterFileReader
::GetOption( const std::string & key, const std::string & defaultValue) const
{
  if (this->HasOption( key ))
    return m_Options.find( key )->second;
  else
    return defaultValue;
}

bool
RuntimeParameterFileReader
::GetOptionAsBool(const std::string & key, bool defaultValue) const
{
  if (! (this->HasOption( key )))
    return defaultValue;
  const std::string & Option = m_Options.find( key )->second;
  if (Option == "1") // for fastest results, use 0 and 1.
    return true;
  if (Option == "0")
    return false;
  std::string value = Option;
  std::transform( Option.begin(), Option.end(), value.begin(), ::tolower );
  if ((value == "true") || (value == "yes") || (value == "on"))
    return true;
  if ((value == "false") || (value == "no") || (value == "off"))
    return false;
  return defaultValue;
}

double
RuntimeParameterFileReader
::GetOptionAsDouble(const std::string & key) const
{
  return std::atof(this->GetOption( key ).c_str());
}

double
RuntimeParameterFileReader
::GetOptionAsDouble(const std::string & key, double defaultValue) const
{
  if (this->HasOption( key ))
    return std::atof(this->GetOption( key ).c_str());
  else
    return defaultValue;
}

int
RuntimeParameterFileReader
::GetOptionAsInt(const std::string & key) const
{
  return std::atoi(this->GetOption( key ).c_str());
}

int
RuntimeParameterFileReader
::GetOptionAsInt(const std::string & key, int defaultValue) const
{
  if (this->HasOption( key ))
    return std::atoi(this->GetOption( key ).c_str());
  else
    return defaultValue;
}

long
RuntimeParameterFileReader
::GetOptionAsLong(const std::string & key) const
{
  return std::atol(this->GetOption( key ).c_str());
}

long
RuntimeParameterFileReader
::GetOptionAsLong(const std::string & key, long defaultValue) const
{
  if (this->HasOption( key ))
    return std::atol(this->GetOption( key ).c_str());
  else
    return defaultValue;
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

std::string
RuntimeParameterFileReader
::RegularizeLine( std::string line )
{
  // Chop off anything after comment character
  size_t commentPosition = line.find_first_of( '#' );
  line = line.substr( 0, commentPosition );

  boost::algorithm::trim( line );

  return line;
}

} // end namespace madai
