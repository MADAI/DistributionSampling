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
  RuntimeParameterFileReader();
  ~RuntimeParameterFileReader();

  /**
   * Read a parameters file and initialize the options.
   */
  bool ParseFile( const std::string fileName );

  /**
   * Get an option value.
   */
  const std::string & GetOption(const std::string & key) const;

  /**
   * Get an option value.
   *
   * If this option isn't available, returns defaultValue.
   */
  const std::string & GetOption(
      const std::string & key, const std::string & defaultValue) const;

  /**
   * Get an option value as a double.
   *
   * It is up to the caller to call HasOption(key) before calling
   * this.
   */
  double GetOptionAsDouble(const std::string & key) const;

  /**
   * Get an option value as a double.
   *
   * If this option isn't available, returns defaultValue.
   */
  double GetOptionAsDouble(const std::string & key, double defaultValue) const;

  /**
   * Get an option value as an int.
   *
   * It is up to the caller to call HasOption(key) before calling
   * this.
   */
  int GetOptionAsInt(const std::string & key) const;

  /**
   * Get an option value as an int.
   *
   * If this option isn't available, returns defaultValue.
   */
  int GetOptionAsInt(const std::string & key, int defaultValue) const;

  /**
   * Get an option value as a long.
   *
   * It is up to the caller to call HasOption(key) before calling
   * this.
   */
  long GetOptionAsLong(const std::string & key) const;

  /**
   * Get an option value as a long.
   *
   * If this option isn't available, returns defaultValue.
   */
  long GetOptionAsLong(const std::string & key, long defaultValue) const;

  /**
   * Get an option value as a bool.
   *
   * If the option doesn't exist or is not one of the values listed
   * below, returns the default value.
   *
   * Values evaluating to "true": 1, true, TRUE, True, yes, YES, on, On, or ON.
   *
   * Values evaluating to "false": 0, false FALSE, False, NO, no, off,
   * Off, or OFF.
   */
  bool GetOptionAsBool(const std::string & key, bool defaultValue=false) const;

  /**
   * Check to see if an option with the name "key" is specified. */
  bool HasOption(const std::string & key) const;

  /**
   * Print options to an output stream. Used for debugging. */
  void PrintAllOptions(std::ostream & out) const;

  /**
   * Get all options as a std::map that maps from the option name to
   * the option value. */
  const std::map<std::string, std::string> GetAllOptions() const;

private:
  /** Map between option name and value. */
  std::map<std::string, std::string> m_Options;

  /** Strips away comments and leading and trailing spaces from the
   *  line. */
  std::string RegularizeLine( std::string line );

}; // end class RuntimeParameterFileReader

} // end namespace madai
#endif
