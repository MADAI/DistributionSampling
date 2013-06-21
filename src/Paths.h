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

  extern const char SEPARATOR;

  extern const std::string TRACE_DIRECTORY;

  extern const std::string OBSERVABLE_NAMES_FILE;

  extern const std::string PARAMETERS_FILE;

  extern const std::string RUNTIME_PARAMETER_FILE;

  extern const std::string RESULTS_FILE;

  extern const std::string EMULATOR_STATE_FILE;

  extern const std::string PCA_DECOMPOSITION_FILE;

  extern const std::string PARAMETER_PRIORS_FILE;

} // end namespace Paths

} // end namespace madai

#endif // madai_Paths_h_included
