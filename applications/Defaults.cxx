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

#include "Defaults.h"

namespace madai {

const std::string Defaults::MODEL_OUTPUT_DIRECTORY = "model_output";

const std::string Defaults::EXPERIMENTAL_RESULTS_FILE =
  "experimental_results.dat";

const bool Defaults::VERBOSE  = false;

const bool Defaults::READER_VERBOSE = false;

const int Defaults::GENERATE_TRAINING_POINTS_NUMBER_OF_POINTS  = 100;

const bool Defaults::GENERATE_TRAINING_POINTS_PARTITION_BY_PERCENTILE = true;

const double Defaults::GENERATE_TRAINING_POINTS_STANDARD_DEVIATIONS = 3.0;

const bool Defaults::GENERATE_TRAINING_POINTS_USE_MAXIMIN = false;

const int Defaults::GENERATE_TRAINING_POINTS_MAXIMIN_TRIES = 20;

const int Defaults::PERCENTILE_GRID_NUMBER_OF_SAMPLES = 100;

const double Defaults::PCA_FRACTION_RESOLVING_POWER = 0.95;

const std::string Defaults::EMULATOR_COVARIANCE_FUNCTION =
    "SQUARE_EXPONENTIAL_FUNCTION";

const int Defaults::EMULATOR_REGRESSION_ORDER = 1;

const double Defaults::EMULATOR_NUGGET = 0.001;

const double Defaults::EMULATOR_AMPLITUDE = 1.0;

const double Defaults::EMULATOR_SCALE  = 0.01;

const bool Defaults::MCMC_USE_MODEL_ERROR = false;

const int Defaults::MCMC_NUMBER_OF_SAMPLES = 100;

const int Defaults::MCMC_NUMBER_OF_BURN_IN_SAMPLES = 0;

const double Defaults::MCMC_STEP_SIZE = 0.1;

const std::string Defaults::EXTERNAL_MODEL_EXECUTABLE = "";

const std::string Defaults::EXTERNAL_MODEL_ARGUMENTS = "";

const bool Defaults::EMULATE_QUIET = false;

const bool Defaults::EMULATE_WRITE_HEADER = true;

} // end namespace madai
