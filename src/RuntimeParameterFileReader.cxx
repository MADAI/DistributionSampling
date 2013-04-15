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

namespace madai {

RuntimeParameterFileReader
::RuntimeParameterFileReader()
{
}

RuntimeParameterFileReader
::~RuntimeParameterFileReader()
{
}

char*
RuntimeParameterFileReader
::ParseFile( const std::string fileName )
{
  std::ifstream inFile( fileName.c_str() );
  std::string element;
  std::string arg_list;
  while ( inFile >> element ) { arg_list += element + " "; }
  char* Arguments = new char[std::strlen(arg_list.c_str())+1]();
  std::strcpy( Arguments, arg_list.c_str() );
  return Arguments;
}

} // end namespace madai