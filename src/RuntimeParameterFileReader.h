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

#ifndef madai_RuntimeParameterFileReader_h_included
#define madai_RuntimeParameterFileReader_h_included

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <map>

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

  /**
   Get an option value */
  const std::string & GetOption(const std::string & key) const;

  /**
   Get an option value.
   If (! this->HasOption(key)) return defaultValue; */
  const std::string & GetOption(
      const std::string & key, const std::string & defaultValue) const;

  /**
   Get an option value cast to double
   It is up to the user to call HasOption(key) before calling this.  */
  double GetOptionAsDouble(const std::string & key) const;

  /**
   Get an option value cast to double.
   If (! this->HasOption(key)) return defaultValue; */
  double GetOptionAsDouble(const std::string & key, double defaultValue) const;

  /**
   Get an option value cast to integer
   It is up to the user to call HasOption(key) before calling this.  */
  long GetOptionAsInt(const std::string & key) const;

  /**
   Get an option value cast to integer
   It is up to the user to call HasOption(key) before calling this.  */
  long GetOptionAsInt(const std::string & key, long defaultValue) const;

  /**
   Get an option value as a bool.
   If (! this->HasOption(key)
     return defaultValue.
   If this->GetOption(key) is 1, true, TRUE, True, yes, YES, on, On, or ON,
     return true
   If this->GetOption(key) is 0, false FALSE, False, NO, no, off, Off, or OFF,
     return false
   Otherwise behavior is undefined.
  */
  bool GetOptionAsBool(const std::string & key, bool defaultValue=false) const;

  /**
   Check to see if an option is specified. */
  bool HasOption(const std::string & key) const;

  /**
   Used for debugging. */
  void PrintAllOptions(std::ostream & out) const;

  /**
   Used for unit tests. */
  const std::map<std::string, std::string> GetAllOptions() const;

private:
  std::map<std::string, std::string> m_Options;

  // Converts line to format "<name> <value1> <value1> ...\n"
  std::string RegularizeLine( std::string line );

}; // end class RuntimeParameterFileReader

} // end namespace madai
#endif
