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

#ifndef madai_System_h_included
#define madai_System_h_included

#include <string>


namespace madai {

class System {
public:

  /** Returns true if the path points to a file, false if it doesn't exist, and
   * false if it is a directory. */
  static bool IsFile( const char * path );
  static bool IsFile( const std::string & path );

  /** Returns true if the path points to a directory, false if it points
   * to a file, false if it doesn't exist. */
  static bool IsDirectory( const char * path );
  static bool IsDirectory( const std::string & path );

};

} // end namespace madai

#endif // madai_System_h_included
