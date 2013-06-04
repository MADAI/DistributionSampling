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

class Defaults {
public:
  /**
     Generic Variables */
  static const std::string MODEL_OUTPUT_DIRECTORY;

  static const std::string EXPERIMENTAL_RESULTS_FILE;

  static const bool VERBOSE;

  static const bool READER_VERBOSE;

  /**
   Latin Hypercube Variables */
  static const int GENERATE_TRAINING_POINTS_NUMBER_OF_POINTS;

  static const bool GENERATE_TRAINING_POINTS_PARTITION_BY_PERCENTILE;

  static const double GENERATE_TRAINING_POINTS_STANDARD_DEVIATIONS;

  static const bool GENERATE_TRAINING_POINTS_USE_MAXIMIN;

  static const int GENERATE_TRAINING_POINTS_MAXIMIN_TRIES;

  static const int PERCENTILE_GRID_NUMBER_OF_SAMPLES;

  /**
   Emulator Variables */
  static const double PCA_FRACTION_RESOLVING_POWER;

  static const std::string EMULATOR_COVARIANCE_FUNCTION;

  static const int EMULATOR_REGRESSION_ORDER;

  static const double EMULATOR_NUGGET;

  static const double EMULATOR_AMPLITUDE;

  static const double EMULATOR_SCALE;

  static const std::string EMULATOR_TRAINING_RIGOR;

  /**
   Sampler variables */
  static const std::string SAMPLER;

  /**
   MCMC Variables */
  static const bool MCMC_USE_MODEL_ERROR;

  static const int MCMC_NUMBER_OF_SAMPLES;

  static const int MCMC_NUMBER_OF_BURN_IN_SAMPLES;

  static const double MCMC_STEP_SIZE;

  /**
   External Model Variables */
  static const std::string EXTERNAL_MODEL_EXECUTABLE;

  static const std::string EXTERNAL_MODEL_ARGUMENTS;

  /**
   Emulate Variables */
  static const bool EMULATE_QUIET;

  static const bool EMULATE_WRITE_HEADER;

  /**
    Print All Defaults */
  static void PrintAllDefaults(std::ostream & output);
};


} // end namespace madai

#endif // madai_DirectoryDefaults_h_included
