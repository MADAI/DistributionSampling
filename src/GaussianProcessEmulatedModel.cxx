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

#include "GaussianProcessEmulatedModel.h"

#include "GaussianProcessEmulatorDirectoryReader.h"
#include "GaussianProcessEmulatorSingleFileReader.h"

#include <fstream>


namespace madai {


GaussianProcessEmulatedModel
::GaussianProcessEmulatedModel()
{
  m_StateFlag = UNINITIALIZED;
}

GaussianProcessEmulatedModel
::~GaussianProcessEmulatedModel()
{
  // nothing to do.
}

/**
 * Loads a configuration from a file.  The format of the file is
 * defined by this function.  We'll lock it down later.
 */
Model::ErrorType
GaussianProcessEmulatedModel
::LoadConfigurationFile( const std::string fileName )
{
  std::ifstream input(fileName.c_str());
  if (! input.good())
    return Model::FILE_NOT_FOUND_ERROR;
  GaussianProcessEmulatorSingleFileReader singleFileReader;
  if (! singleFileReader.Load(&m_GPME, input))
    return Model::OTHER_ERROR;
  if (m_GPME.m_Status != GaussianProcessEmulator::READY)
    return Model::OTHER_ERROR;
  m_StateFlag = READY;
  // std::vector<T>::operator=(const std::vector<T>&)
  m_Parameters = m_GPME.m_Parameters;
  m_ScalarOutputNames = m_GPME.m_OutputNames;

  int t = m_GPME.m_NumberOutputs;
  std::vector< double > observedScalarValues(t, 0.0);
  std::vector< double > observedUncertaintyScales(t, 0.0);
  m_GPME.GetOutputObservedValues(observedScalarValues);
  m_GPME.GetOutputUncertaintyScales(observedUncertaintyScales);
  std::vector< double > observedScalarCovariance(t * t, 0.0);
  for (int i = 0; i < t; ++i) {
    observedScalarCovariance[i * (1 + t)]
      = std::pow(observedUncertaintyScales[i],2);
  }
  Model::ErrorType error;
  error = this->SetObservedScalarValues(observedScalarValues);
  if (error != Model::NO_ERROR) {
    std::cerr << "Error in Model::SetObservedScalarValues()";
    return error;
  }
  error = this->SetObservedScalarCovariance(observedScalarCovariance);
  if (error != Model::NO_ERROR) {
    std::cerr << "Error in Model::SetObservedScalarCovariance()";
    return error;
  }
  return Model::NO_ERROR;
}

/**
 * Loads a configuration from a directory structure. The format of the file
 * is defined by this function.
 */
Model::ErrorType
GaussianProcessEmulatedModel
::LoadConfiguration( const std::string StatisticsDirectory,
                     const std::string ModelOutputDirectory,
                     const std::string ExperimentalResultsDirectory )
{
  if ( StatisticsDirectory == "-" ) {
    GaussianProcessEmulatorSingleFileReader singleFileReader;
    singleFileReader.Load(&m_GPME, std::cin);
  } else {
    GaussianProcessEmulatorDirectoryReader directoryReader;
    if ( !directoryReader.LoadTrainingData( &m_GPME, ModelOutputDirectory,
                                           StatisticsDirectory, ExperimentalResultsDirectory ) ) {
      std::cerr << "Error loading from the directory structure.\n";
      return Model::OTHER_ERROR;
    }
    if ( !directoryReader.LoadPCA( &m_GPME, StatisticsDirectory ) ) {
      std::cerr << "Error loading the PCA decomposition data.\n";
      return Model::OTHER_ERROR;
    }
    if ( !directoryReader.LoadEmulator( &m_GPME, StatisticsDirectory ) ) {
      std::cerr << "Error loading Emulator data.\n";
      return Model::OTHER_ERROR;
    }
  }
  if ( m_GPME.m_Status != GaussianProcessEmulator::READY )
    return Model::OTHER_ERROR;
  m_StateFlag = READY;
  m_Parameters = m_GPME.m_Parameters;
  m_ScalarOutputNames = m_GPME.m_OutputNames;
  return Model::NO_ERROR;
}

/**
 * Set the gaussian process emulator
 */
Model::ErrorType
GaussianProcessEmulatedModel
::SetGaussianProcessEmulator(
  GaussianProcessEmulator & GPME )
{
  m_GPME = GPME;
  if ( m_GPME.m_Status != GaussianProcessEmulator::READY )
    return Model::OTHER_ERROR;
  m_StateFlag = READY;
  m_Parameters = m_GPME.m_Parameters;
  m_ScalarOutputNames = m_GPME.m_OutputNames;
  return Model::NO_ERROR;
}

/**
   Returns a const reference to internal data for debugging purposes. */
const GaussianProcessEmulator &
GaussianProcessEmulatedModel
::GetGaussianProcessEmulator() const
{
  return m_GPME;
}

/**
 * Get the scalar outputs from the model evaluated at x.  If an
 * error happens, the scalar output array will be left incomplete.
 */
Model::ErrorType
GaussianProcessEmulatedModel
::GetScalarOutputsAndCovariance(
  const std::vector< double > & parameters,
  std::vector< double > & scalars,
  std::vector< double > & scalarCovariance) const
{
  if (m_GPME.m_Status != GaussianProcessEmulator::READY)
    return Model::OTHER_ERROR;

  if (this->GetNumberOfParameters() != parameters.size())
    return Model::OTHER_ERROR;

  if (! m_GPME.GetEmulatorOutputsAndCovariance(
          parameters, scalars, scalarCovariance))
    return Model::OTHER_ERROR;

  return Model::NO_ERROR;
}

/**
 * Get the scalar outputs from the model evaluated at a position in
 * the parameter space.
 */
Model::ErrorType
GaussianProcessEmulatedModel
::GetScalarOutputs(
  const std::vector< double > & parameters,
  std::vector< double > & scalars ) const
{
  if (m_GPME.m_Status != GaussianProcessEmulator::READY)
    return Model::OTHER_ERROR;

  if (this->GetNumberOfParameters() != parameters.size())
    return Model::OTHER_ERROR;

  if (! m_GPME.GetEmulatorOutputs (parameters, scalars))
    return Model::OTHER_ERROR;

  return Model::NO_ERROR;
}

} // namespace madai
