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

#ifndef madai_RuntimeParameterFileReader_h_included
#define madai_RuntimeParameterFileReader_h_included

#include <cstdlib>
#include <iostream>
#include <fstream>

namespace madai {

/** \class RuntimeParameterFileReader
 *
 * Reads runtime parameters for applications. */
class RuntimeParameterFileReader {
public:
  // Constructor
  RuntimeParameterFileReader();

  // Destructor
  ~RuntimeParameterFileReader();

  // Read a file and output its contents as a character string
  bool ParseFile( const std::string fileName );

  // Get number of arguments
  int GetNumberOfArguments();

  // Get arguments themselves
  char ** GetArguments();

private:
  int    m_NumberOfArguments;
  char** m_Arguments;

  void FreeMemory();
}; // end class RuntimeParameterFileReader

}; // end namespace madai
#endif
