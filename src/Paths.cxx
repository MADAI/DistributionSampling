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

#include "Paths.h"

namespace madai {

#ifdef WIN32
const char Paths::SEPARATOR = '\\';
#else
const char Paths::SEPARATOR = '/';
#endif

const std::string Paths::DEFAULT_MODEL_OUTPUT_DIRECTORY( "model_output" );

const std::string Paths::DEFAULT_EXPERIMENTAL_RESULTS_DIRECTORY( "experimental_results" );

const std::string Paths::TRACE_DIRECTORY( "trace" );

const std::string Paths::OBSERVABLE_NAMES_FILE( "observable_names.dat" );

const std::string Paths::PARAMETERS_FILE( "parameters.dat" );

const std::string Paths::RESULTS_FILE( "results.dat" );

const std::string Paths::RUNTIME_PARAMETER_FILE( "stat_params.dat" );

const std::string Paths::EMULATOR_STATE_FILE( "emulator_state.dat" );

const std::string Paths::PCA_DECOMPOSITION_FILE( "pca_decomposition.dat" );

const std::string Paths::PARAMETER_PRIORS_FILE( "parameter_priors.dat" );

} // end namespace madai
