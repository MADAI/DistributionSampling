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
#include <iostream> // cout
#include "Defaults.h" // madai::Defaults
int main(int argc, char ** argv) {
  (void)argc; (void)argv;
  std::cout
    << "#\n"
    << "MODEL_OUTPUT_DIRECTORY "
    << madai::Defaults::MODEL_OUTPUT_DIRECTORY << '\n'
    << "EXPERIMENTAL_RESULTS_FILE "
    << madai::Defaults::EXPERIMENTAL_RESULTS_FILE << '\n'
    << "#\n"
    << "VERBOSE "
    << madai::Defaults::VERBOSE << '\n'
    << "READER_VERBOSE "
    << madai::Defaults::READER_VERBOSE << '\n'
    << "#\n"
    << "GENERATE_TRAINING_POINTS_NUMBER_OF_POINTS "
    << madai::Defaults::GENERATE_TRAINING_POINTS_NUMBER_OF_POINTS << '\n'
    << "GENERATE_TRAINING_POINTS_PARTITION_BY_PERCENTILE "
    << madai::Defaults::GENERATE_TRAINING_POINTS_PARTITION_BY_PERCENTILE << '\n'
    << "GENERATE_TRAINING_POINTS_STANDARD_DEVIATIONS "
    << madai::Defaults::GENERATE_TRAINING_POINTS_STANDARD_DEVIATIONS << '\n'
    << "#\n"
    << "PERCENTILE_GRID_NUMBER_OF_SAMPLES "
    << madai::Defaults::PERCENTILE_GRID_NUMBER_OF_SAMPLES << '\n'
    << "#\n"
    << "PCA_FRACTION_RESOLVING_POWER "
    << madai::Defaults::PCA_FRACTION_RESOLVING_POWER << '\n'
    << "EMULATOR_COVARIANCE_FUNCTION "
    << madai::Defaults::EMULATOR_COVARIANCE_FUNCTION << '\n'
    << "EMULATOR_REGRESSION_ORDER "
    << madai::Defaults::EMULATOR_REGRESSION_ORDER << '\n'
    << "EMULATOR_NUGGET "
    << madai::Defaults::EMULATOR_NUGGET << '\n'
    << "EMULATOR_AMPLITUDE "
    << madai::Defaults::EMULATOR_AMPLITUDE << '\n'
    << "EMULATOR_SCALE "
    << madai::Defaults::EMULATOR_SCALE << '\n'
    << "#\n"
    << "MCMC_USE_MODEL_ERROR "
    << madai::Defaults::MCMC_USE_MODEL_ERROR << '\n'
    << "MCMC_NUMBER_OF_SAMPLES "
    << madai::Defaults::MCMC_NUMBER_OF_SAMPLES << '\n'
    << "MCMC_NUMBER_OF_BURN_IN_SAMPLES "
    << madai::Defaults::MCMC_NUMBER_OF_BURN_IN_SAMPLES << '\n'
    << "MCMC_STEP_SIZE "
    << madai::Defaults::MCMC_STEP_SIZE << '\n'
    << "#\n"
    << "EXTERNAL_MODEL_EXECUTABLE "
    << madai::Defaults::EXTERNAL_MODEL_EXECUTABLE << '\n'
    << "EXTERNAL_MODEL_ARGUMENTS "
    << madai::Defaults::EXTERNAL_MODEL_ARGUMENTS << '\n'
    << "#\n"
    << "EMULATE_QUIET "
    << madai::Defaults::EMULATE_QUIET << '\n'
    << "EMULATE_WRITE_HEADER "
    << madai::Defaults::EMULATE_WRITE_HEADER << '\n'
    << "#\n";
  return 0;
}
