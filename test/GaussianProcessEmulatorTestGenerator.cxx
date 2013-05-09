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

#include "GaussianProcessEmulatorTestGenerator.h"

#include "LatinHypercubeGenerator.h"
#include "Random.h"
#include "Sample.h"
#include "UniformDistribution.h"
#include "Paths.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include "GaussianProcessEmulatorDirectoryReader.h"

#include "madaisys/SystemTools.hxx"


GaussianProcessEmulatorTestGenerator
::GaussianProcessEmulatorTestGenerator(
    void (*model)(const std::vector< double > &, std::vector< double > &),
    int numberParameters,
    int numberOutputs,
    int numberTrainingPoints,
    const std::vector< madai::Parameter > & parameters ) :
  m_NumberParameters(numberParameters),
  m_NumberOutputs(numberOutputs),
  m_NumberTrainingPoints(numberTrainingPoints),
  m_Parameters( parameters )
{
  assert (parameters.size() >= static_cast< size_t >( numberParameters ));
  assert((numberTrainingPoints > 0) && (numberParameters > 0));

  madai::LatinHypercubeGenerator latinHypercubeGenerator;
  std::vector< madai::Sample > samples =
    latinHypercubeGenerator.Generate( numberTrainingPoints, parameters );

  m_X.resize(numberTrainingPoints,numberParameters);
  m_Y.resize(numberTrainingPoints,numberOutputs);

  std::vector< double > x(numberParameters+4,0.0);
  std::vector< double > y(numberOutputs+4,0.0);
  
  for ( int i = 0; i < m_NumberOutputs; ++i ) {
    std::stringstream ss;
    ss << "output_" << i;
    m_OutputNames.push_back( ss.str() );
  }

  for(int i = 0; i < numberTrainingPoints; ++i) {
    for(int j = 0; j < numberParameters; ++j) {
      m_X(i,j) = samples[i].m_ParameterValues[j];
      x[j] = m_X(i,j);
    }
    (*model)(x,y);
    for(int j = 0; j < numberOutputs; ++j) {
      m_Y(i,j) = y[j];
    }
  }
}

bool
GaussianProcessEmulatorTestGenerator
::WriteDirectoryStructure( std::string StatisticsDirectory )
{
  // Write parameter priors file
  std::string priorFile = StatisticsDirectory + madai::Paths::SEPARATOR
      + madai::Paths::PARAMETER_PRIORS_FILE;
  std::ofstream ParametersPriorFile( priorFile.c_str() );
  if ( !ParametersPriorFile ) {
    std::cerr << "Could not open file '" << priorFile << "'\n";
    return false;
  }

  for ( int i = 0; i < m_NumberParameters; ++i ) {
    const madai::Distribution * distribution = m_Parameters[i].GetPriorDistribution();
    const madai::UniformDistribution * uniformDistribution = 
      dynamic_cast< const madai::UniformDistribution * >( distribution );
    ParametersPriorFile << "uniform " << m_Parameters[ i ].m_Name << " "
                        << uniformDistribution->GetMinimum() << " " 
                        << uniformDistribution->GetMaximum() << "\n";
  }
  ParametersPriorFile.close();

  // Create the observable names file
  std::string observablesFileName = StatisticsDirectory + madai::Paths::SEPARATOR
      + madai::Paths::OBSERVABLE_NAMES_FILE;
  std::ofstream ObservablesFile( observablesFileName.c_str() );
  if ( !ObservablesFile ) {
    std::cerr << "Could not open file '" << observablesFileName << "'\n";
    return false;
  }
  
  for ( int i = 0; i < m_NumberOutputs; ++i ) {
    ObservablesFile << m_OutputNames[i] << "\n";
  }
  ObservablesFile.close();
  
  // Create the model output directory
  std::string modelOutputDirectory = StatisticsDirectory + madai::Paths::SEPARATOR
      + madai::Paths::DEFAULT_MODEL_OUTPUT_DIRECTORY;
  bool directoryCreated = madaisys::SystemTools::MakeDirectory( modelOutputDirectory.c_str() );
  if ( !directoryCreated ) {
    std::cerr << "Could not create directory '" << modelOutputDirectory << "'\n";
    return false;
  }
  
  // Create the run directories and write the results.dat and parameters.dat files
  assert( m_Y.rows() == m_X.rows() );
  for ( int i = 0; i < m_Y.rows(); ++i ) {
    std::ostringstream buffer;
    buffer << modelOutputDirectory << "/run"
      << std::setw( 4) << std::setfill( '0' ) << i;
    std::string runDirectory(buffer.str());
    
    directoryCreated = madaisys::SystemTools::MakeDirectory( runDirectory.c_str() );
    if ( !directoryCreated ) {
      std::cerr << "Could not create directory '" << runDirectory << "\n";
      return false;
    }
    
    std::string parametersFile( runDirectory + madai::Paths::SEPARATOR +
                                madai::Paths::PARAMETERS_FILE );
    std::ofstream ParameterOutfile ( parametersFile.c_str() );
    if ( !ParameterOutfile ) {
      std::cerr << "Could not open file '" << parametersFile << "'\n";
      return false;
    }

    for ( int j = 0; j < m_NumberParameters; j++ ) {
      ParameterOutfile << m_Parameters[j].m_Name << ' ' << m_X( i, j ) << "\n";
    }
    ParameterOutfile.close();
    
    std::string modelOutputFile( runDirectory + madai::Paths::SEPARATOR +
                                 madai::Paths::RESULTS_FILE );
    std::ofstream ResultsFile ( modelOutputFile.c_str() );
    if ( !ResultsFile ) {
      std::cerr << "Coul not open file '" << modelOutputFile << "'\n";
      return false;
    }
    
    for ( int j = 0; j < m_NumberOutputs; ++j ) {
      ResultsFile << m_OutputNames[j] << ' ' << m_Y( i, j ) << "\n";
    }
    ResultsFile.close();
  }
  
  // Create an experimental results directory
  std::string ExperimentalResultsDir = StatisticsDirectory + madai::Paths::SEPARATOR
      + "experimental_results";
  directoryCreated = madaisys::SystemTools::MakeDirectory( ExperimentalResultsDir.c_str() );
  if ( !directoryCreated ) {
    std::cerr << "Could not create directory '" << ExperimentalResultsDir << "'\n";
    return false;
  }
  
  // Write the experimental results file
  std::string ExperimentalResultsFile = ExperimentalResultsDir + madai::Paths::SEPARATOR
      + madai::Paths::DEFAULT_EXPERIMENTAL_RESULTS_FILE;
  std::ofstream ExpResultsFile( ExperimentalResultsFile.c_str() );
  if ( !ExpResultsFile ) {
    std::cerr << "Could not open file '" << ExperimentalResultsFile << "'\n";
    return false;
  }
  
  // Simply put the values to the middle of the range
  for ( int i = 0; i < m_NumberOutputs; ++i ) {
    const madai::Distribution * distribution = m_Parameters[i].GetPriorDistribution();
    const madai::UniformDistribution * uniformDistribution = 
      dynamic_cast< const madai::UniformDistribution * >( distribution );
    double min = uniformDistribution->GetMinimum();
    double max = uniformDistribution->GetMaximum();
    double value = min + (max - min)/2.0;
    ExpResultsFile << m_OutputNames[i] << " " << value << " " << 0.01 << "\n";
  }
  ExpResultsFile.close();
  
  return true;
}

void
GaussianProcessEmulatorTestGenerator
::WriteTrainingFile(std::ostream & o)
{
  o.precision(17);
  o << "#\n#\n#\nVERSION 1\nPARAMETERS\n"<< m_NumberParameters << "\n";
  for(int i = 0; i < m_NumberParameters; ++i) {
    const madai::Distribution * distribution = m_Parameters[i].GetPriorDistribution();
    const madai::UniformDistribution * uniformDistribution =
      dynamic_cast< const madai::UniformDistribution * >( distribution );
    o << "param_" << i << " UNIFORM " << uniformDistribution->GetMinimum()
      << " " << uniformDistribution->GetMaximum() << "\n";
  }
  o << "OUTPUTS\n" << m_NumberOutputs << "\n";
  for(int i = 0; i < m_NumberOutputs; ++i)
    o << "output_" << i << "\n";
  o << "NUMBER_OF_TRAINING_POINTS\n" << m_NumberTrainingPoints << "\n";
  o << "PARAMETER_VALUES\n" << m_NumberTrainingPoints
    << " " << m_NumberParameters << "\n" << m_X << "\n";
  o << "OUTPUT_VALUES\n" << m_NumberTrainingPoints
    << " " << m_NumberOutputs << "\n" << m_Y << "\nEND_OF_FILE\n";
  o.flush();
}
