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

#include "GaussianProcessEmulatedModel.h"

namespace madai {

GaussianProcessEmulatedModel
::GaussianProcessEmulatedModel() :
  m_Emulator(NULL)
{
  m_StateFlag = UNINITIALIZED;
  // Note that all methods must check for (m_Emulator == NULL).
}

GaussianProcessEmulatedModel
::~GaussianProcessEmulatedModel()
{
  delete m_Emulator;
}

/**
 * Loads a configuration from a file.  The format of the file is
 * defined by this function.  We'll lock it down later.
 */
Model::ErrorType
GaussianProcessEmulatedModel
::LoadConfigurationFile( const std::string fileName )
{
  m_Emulator = new emulator(fileName);
  if (! m_Emulator->IsOkay()) {
     delete m_Emulator;
     m_Emulator = NULL;
     m_StateFlag = ERROR;
     return Model::OTHER_ERROR;
  }
  m_StateFlag = READY;
  int numberOfParameters = m_Emulator->getNumberOfParameters();
  int numberOfOutputs = m_Emulator->getNumberOfOutputs();
  for (int i = 0; i < numberOfParameters; ++i) {
    std::string name(m_Emulator->getParameterName(i));
    double min = m_Emulator->getParameterMinimum(i);
    double max = m_Emulator->getParameterMaximum(i);
    this->AddParameter(name, min, max);
  }
  for (int i = 0; i < numberOfOutputs; ++i) {
    std::string name(m_Emulator->getOutputName(i));
    this->AddScalarOutputName(name );
  }
  return Model::NO_ERROR;
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
  if (m_Emulator == NULL)
    return Model::OTHER_ERROR;

  emulator * emu = const_cast<emulator * >( m_Emulator );

  if (this->GetNumberOfParameters() != parameters.size())
    return Model::OTHER_ERROR;

  emu->QueryEmulator(parameters, scalars, scalarCovariance);
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
  std::vector< double > scalarCovariance;
  return this->GetScalarOutputsAndCovariance(
    parameters, scalars, scalarCovariance);
}

} // namespace madai
