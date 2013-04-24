/*=========================================================================
 *
 *  Copyright 2011-2013 The University of North Carolina at Chapel Hill
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

#include "RuntimeParameterFileReader.h"

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
  if ( inFile ) {
    std::string element;
    std::vector<std::string> arg_list;
    m_NumberOfArguments = 0;
    while ( inFile.good() ) { 
      while ( inFile.peek() == '#' ) {
        std::string line;
        std::getline( inFile, line );
      }
      if ( !(inFile >> element) ) break;
      arg_list.push_back(element); 
    }
    m_NumberOfArguments = arg_list.size();
    m_Arguments = new char*[m_NumberOfArguments]();
    for ( unsigned int i = 0; i < m_NumberOfArguments; i++ ) {
      m_Arguments[i] = new char[std::strlen(arg_list[i].c_str())+1];
      std::strcpy( m_Arguments[i], arg_list[i].c_str() );
    }

    inFile.seekg(0, std::ios::beg);
    while ( inFile.good() ) {
      std::string line;
      while ( inFile.peek() == '#' ) {
        std::getline( inFile, line );
      }
      std::getline( inFile, line );
      size_t index = line.find(' ');
      if (index != std::string::npos) {
        std::string key = line.substr(0,index);
        m_Options[key] = line.substr(index + 1);
      }
    }
  } else {
    std::cerr << "RuntimeParameterFileReader couldn't find input file'" << fileName << "'\n";
    return false;
  }

  return true;
}

bool
RuntimeParameterFileReader
::HasOption(const std::string & key)
{
  return (m_Options.count(key) > 0);
}

const std::string &
RuntimeParameterFileReader
::GetOption(const std::string & key)
{
  static const std::string empty("");
  if (this->HasOption(key))
    return m_Options[key];
  else
    return empty;
}



int
RuntimeParameterFileReader
::GetNumberOfArguments()
{
  return m_NumberOfArguments;
}

char **
RuntimeParameterFileReader
::GetArguments()
{
  return m_Arguments;
}

void
RuntimeParameterFileReader
::FreeMemory()
{
  for ( unsigned int i = 0; i < m_NumberOfArguments; i++ ) {
    delete[] m_Arguments[i];
  }
  delete[] m_Arguments;

  m_Arguments = NULL;
}

} // end namespace madai
