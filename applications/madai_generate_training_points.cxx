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

#include <iostream> // std::cerr
#include <fstream> // std::ifstream, std::ofstream
#include <string> // std::string
#include <vector> // std::vector
#include <sstream> // std::ostringstream

#include "ApplicationUtilities.h"
#include "GaussianDistribution.h"
#include "GaussianProcessEmulatorDirectoryReader.h"
#include "LatinHypercubeGenerator.h"
#include "Parameter.h"
#include "Paths.h"
#include "Defaults.h"
#include "RuntimeParameterFileReader.h"
#include "UniformDistribution.h"

#include "madaisys/SystemTools.hxx"

using madai::Paths;

bool WriteDirectories( const std::string modelOutputDirectory,
                       const std::vector< madai::Parameter > & parameters,
                       const std::vector< madai::Sample > & samples,
                       const bool verbose ) {
  // Create the directory structure
  bool directoryCreated = madaisys::SystemTools::MakeDirectory( modelOutputDirectory.c_str() );
  if ( !directoryCreated ) {
    std::cerr << "Could not create directory '" << modelOutputDirectory << "'\n";
    return false;
  }

  // Now create the run directories
  for ( size_t i = 0; i < samples.size(); ++i ) {
    const madai::Sample & sample = samples[i];

    std::ostringstream buffer;
    buffer << modelOutputDirectory << "/run"
    << std::setw( 4 ) << std::setfill( '0' ) << i;
    std::string runDirectory(buffer.str());

    if ( verbose ) {
      std::cout << runDirectory << "\n";
    }

    directoryCreated = madaisys::SystemTools::MakeDirectory( runDirectory.c_str() );
    if ( !directoryCreated ) {
      std::cerr << "Could not create directory '" << runDirectory << "\n";
      return false;
    }

    // Write the parameters to parameters.dat file
    std::string parametersFile( runDirectory + Paths::SEPARATOR + Paths::PARAMETERS_FILE );
    std::ofstream outfile( parametersFile.c_str() );
    if ( !outfile ) {
      std::cerr << "Could not open file '" << parametersFile << "'\n";
      return false;
    }

    assert( parameters.size() == sample.m_ParameterValues.size() );
    for ( size_t j = 0; j < parameters.size(); ++j ) {
      outfile << parameters[ j ].m_Name << ' '
              << sample.m_ParameterValues[ j ] << '\n';
    }

    outfile.close();
  }

  return true;
}


int main( int argc, char * argv[] ) {
  if ( argc < 2 ) {
    std::cerr
      << "Usage:\n"
      << "    " << argv[0] << " <StatisticsDirectory>\n"
      << "\n"
      << "This program reads from <StatisticsDirectory>/"
      << Paths::PARAMETER_PRIORS_FILE << " in\n"
      << "and uses the parameter prior distribution to generate a series of\n"
      << "parameter files on which to run a model according to a latin hypercube\n"
      << "sampling pattern.\n"
      << "\n"
      << "The format of the " << Paths::PARAMETER_PRIORS_FILE << " file is:\n"
      << "uniform name min max\n"
      << "gaussian name mean std_dev\n"
      << "\n"
      << "Only uniform and gaussian distributions are available.\n"
      << "\n"
      << "<StatisticsDirectory> is the directory in which all \n"
      << "statistics data are stored. It contains the parameter file "
      << Paths::RUNTIME_PARAMETER_FILE << "\n"
      << "\n"
      << "Format of entries in " << Paths::RUNTIME_PARAMETER_FILE
      << ":\n\n"
      << "MODEL_OUTPUT_DIRECTORY <value> (default: "
      << madai::Defaults::MODEL_OUTPUT_DIRECTORY << ")\n"
      << "GENERATE_TRAINING_POINTS_NUMBER_OF_POINTS <values> (default: "
      << madai::Defaults::GENERATE_TRAINING_POINTS_NUMBER_OF_POINTS << ")\n"
      << "GENERATE_TRAINING_POINTS_PERCENTILE_PARTITION <value> (default: "
      << madai::Defaults::GENERATE_TRAINING_POINTS_PARTITION_BY_PERCENTILE
      << ")\n"
      << "GENERATE_TRAINING_POINTS_STANDARD_DEVIATIONS <value> (default: "
      << madai::Defaults::GENERATE_TRAINING_POINTS_STANDARD_DEVIATIONS << ")\n"
      << "GENERATE_TRAINING_POINTS_USE_MAXIMIN <value> (default: "
      << madai::Defaults::GENERATE_TRAINING_POINTS_USE_MAXIMIN << ")\n"
      << "GENERATE_TRAINING_POINTS_MAXIMIN_TRIES <value> (default: "
      << madai::Defaults::GENERATE_TRAINING_POINTS_MAXIMIN_TRIES << ")\n"
      << "VERBOSE <value> (default: "
      << madai::Defaults::VERBOSE << ")\n";

    return EXIT_FAILURE;
  }

  std::string statisticsDirectory( argv[1] );
  madai::EnsurePathSeparatorAtEnd( statisticsDirectory );

  madai::RuntimeParameterFileReader settings;
  std::string settingsFile = statisticsDirectory + madai::Paths::RUNTIME_PARAMETER_FILE;
  if ( !settings.ParseFile( settingsFile ) ) {
    std::cerr << "Could not open runtime parameter file '" << settingsFile << "'\n";
    return EXIT_FAILURE;
  }

  std::string modelOutputDirectory =
    madai::GetModelOutputDirectory( statisticsDirectory, settings );

  // Read in parameter priors
  std::vector< madai::Parameter > parameters;
  std::string parametersFile = statisticsDirectory + madai::Paths::PARAMETER_PRIORS_FILE;

  bool verbose = settings.GetOptionAsBool(
    "READER_VERBOSE",
    madai::Defaults::READER_VERBOSE );

  int numberOfParameters = 0;
  bool parametersRead = madai::GaussianProcessEmulatorDirectoryReader::
    ParseParameters( parameters, numberOfParameters, statisticsDirectory, false );
  if ( !parametersRead ) {
    std::cerr << "Could not read parameters from prior file '"
              << parametersFile << "'" << std::endl;
    return EXIT_FAILURE;
  }

  double standardDeviations = settings.GetOptionAsDouble(
      "GENERATE_TRAINING_POINTS_STANDARD_DEVIATIONS",
      madai::Defaults::GENERATE_TRAINING_POINTS_STANDARD_DEVIATIONS);

  bool partitionByPercentile = settings.GetOptionAsBool(
      "GENERATE_TRAINING_POINTS_PARTITION_BY_PERCENTILE",
      madai::Defaults::GENERATE_TRAINING_POINTS_PARTITION_BY_PERCENTILE);

  int numberOfTrainingPoints = settings.GetOptionAsInt(
      "GENERATE_TRAINING_POINTS_NUMBER_OF_POINTS",
      madai::Defaults::GENERATE_TRAINING_POINTS_NUMBER_OF_POINTS);

  bool useMaxiMin = settings.GetOptionAsBool(
    "GENERATE_TRAINING_POINTS_USE_MAXIMIN",
    madai::Defaults::GENERATE_TRAINING_POINTS_USE_MAXIMIN );

  int numberOfMaxiMinTries = settings.GetOptionAsInt(
    "GENERATE_TRAINING_POINTS_MAXIMIN_TRIES",
    madai::Defaults::GENERATE_TRAINING_POINTS_MAXIMIN_TRIES );

  // Create the Latin hypercube sampling
  madai::LatinHypercubeGenerator sampleGenerator;
  sampleGenerator.SetStandardDeviations( standardDeviations );
  sampleGenerator.SetPartitionSpaceByPercentile( partitionByPercentile );

  std::vector< madai::Sample > samples;

  if ( useMaxiMin ) {
    samples =
      sampleGenerator.GenerateMaxiMin( numberOfTrainingPoints, parameters,
                                       numberOfMaxiMinTries );
  } else {
    samples =
      sampleGenerator.Generate( numberOfTrainingPoints, parameters );
  }

  if ( !WriteDirectories( modelOutputDirectory, parameters, samples, false ) ) {
    std::cerr << "Could not write model output directory '" << modelOutputDirectory << "'.\n";
    return EXIT_FAILURE;
  }

  if ( settings.GetOptionAsBool( "VERBOSE", madai::Defaults::VERBOSE ) ) {
    std::cout << "Write model output directory '" << modelOutputDirectory << "'.\n";
  }

  return EXIT_SUCCESS;
}
