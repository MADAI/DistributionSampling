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

#ifndef madai_Defaults_h_included
#define madai_Defaults_h_included

#include <string>
#include <iostream>

namespace madai {

namespace Defaults {
  /**
     Generic Variables */
  extern const std::string MODEL_OUTPUT_DIRECTORY;

  extern const std::string EXPERIMENTAL_RESULTS_FILE;

  extern const bool VERBOSE;

  extern const bool READER_VERBOSE;

  /**
   Latin Hypercube Variables */
  extern const int GENERATE_TRAINING_POINTS_NUMBER_OF_POINTS;

  extern const bool GENERATE_TRAINING_POINTS_PARTITION_BY_PERCENTILE;

  extern const double GENERATE_TRAINING_POINTS_STANDARD_DEVIATIONS;

  extern const bool GENERATE_TRAINING_POINTS_USE_MAXIMIN;

  extern const int GENERATE_TRAINING_POINTS_MAXIMIN_TRIES;

  /**
   Emulator Variables */
  extern const double PCA_FRACTION_RESOLVING_POWER;

  extern const std::string EMULATOR_COVARIANCE_FUNCTION;

  extern const int EMULATOR_REGRESSION_ORDER;

  extern const double EMULATOR_NUGGET;

  extern const double EMULATOR_AMPLITUDE;

  extern const double EMULATOR_SCALE;

  extern const std::string EMULATOR_TRAINING_RIGOR;

  /**
   Sampler variables */
  extern const std::string SAMPLER;

  extern const int SAMPLER_NUMBER_OF_SAMPLES;

  extern const std::string SAMPLER_INACTIVE_PARAMETERS_FILE;

  /**
   MCMC Variables */
  extern const bool MCMC_USE_MODEL_ERROR;

  extern const int MCMC_NUMBER_OF_BURN_IN_SAMPLES;

  extern const double MCMC_STEP_SIZE;

  /**
   External Model Variables */
  extern const std::string EXTERNAL_MODEL_EXECUTABLE;

  extern const std::string EXTERNAL_MODEL_ARGUMENTS;

  /**
   Emulate Variables */
  extern const bool EMULATE_WRITE_HEADER;

  /**
   Posterior analysis variables */
  extern const std::string POSTERIOR_ANALYSIS_DIRECTORY;

  /**
    Print All Defaults */
  void PrintAllDefaults(std::ostream & output);
} // end namespace Defaults

} // end namespace madai

#endif // madai_DirectoryDefaults_h_included
