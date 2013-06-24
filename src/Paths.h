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

#ifndef madai_Paths_h_included
#define madai_Paths_h_included

#include <string>

namespace madai {

/** Namespace for paths used in the library. */
namespace Paths {

  /**
   * The path separator for the system. \ for Windows and / for
   * unix-style systems.
   */
  extern const char SEPARATOR;

  /**
   * Directory relative to the working directory where traces are
   * stored.
   */
  extern const std::string TRACE_DIRECTORY;

  /**
   * Name of file containing the names of the observables in the
   * model.
   */
  extern const std::string OBSERVABLE_NAMES_FILE;

  /**
   * Name of the file in each runXXXX directory that contains the
   * parameter values for the model run.
   */
  extern const std::string PARAMETERS_FILE;

  /**
   * The name of the file relative to the working directory that
   * contains the settings.
   */
  extern const std::string RUNTIME_PARAMETER_FILE;

  /**
   * The name of the file in each runXXXX directory that contains the
   * results of a model run.
   */
  extern const std::string RESULTS_FILE;

  /**
   * The name of the file relative to the working directory that
   * contains the emulator state.
   */
  extern const std::string EMULATOR_STATE_FILE;

  /**
   * The name of the file relative to the working directory that
   * contains the principal componenet analysis results.
   */
  extern const std::string PCA_DECOMPOSITION_FILE;

  /**
   * The name of the file containing the parameter names and
   * distributions.
   */
  extern const std::string PARAMETER_PRIORS_FILE;

} // end namespace Paths

} // end namespace madai

#endif // madai_Paths_h_included
