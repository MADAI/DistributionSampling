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

#include "System.h"

#include <madaisys/SystemTools.hxx>

using madaisys::SystemTools;


namespace madai {

bool
System
::IsFile( const char * path )
{
  return ( SystemTools::FileExists( path ) &&
           !SystemTools::FileIsDirectory( path ) );
}


bool
System
::IsFile( const std::string & path )
{
  return IsFile( path.c_str() );
}


bool
System
::IsDirectory( const char * path )
{
  return ( SystemTools::FileExists( path ) &&
           SystemTools::FileIsDirectory( path ) );
}


bool
System
::IsDirectory( const std::string & path )
{
  return IsDirectory( path.c_str() );
}

} // end namespace madai
