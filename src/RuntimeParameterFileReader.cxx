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
  } else {
    std::cerr << "Couldn't find input file\n";
    return false;
  }

  return true;
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
