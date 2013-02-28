/*=========================================================================
 *
 *  Copyright The University of North Carolina at Chapel Hill
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

GaussianProcessEmulatedModel::GaussianProcessEmulatedModel() :
  m_emulator(NULL)
{
	this->m_StateFlag = UNINITIALIZED;
  // Note that all methods must check for (this->m_emulator == NULL).
}

GaussianProcessEmulatedModel::~GaussianProcessEmulatedModel()
{
  if (this->m_emulator != NULL)
    delete this->m_emulator;
}

/**
 * Loads a configuration from a file.  The format of the file is
 * defined by this function.  We'll lock it down later.
 */
Model::ErrorType GaussianProcessEmulatedModel::LoadConfigurationFile( const std::string fileName )
{
  this->m_emulator = new emulator(fileName);
  if (! this->m_emulator->IsOkay()) {
     delete this->m_emulator;
     this->m_emulator = NULL;
     this->m_StateFlag = ERROR;
     return Model::OTHER_ERROR;
  }
  this->m_StateFlag = READY;
  int numberOfParameters = this->m_emulator->getNumberOfParameters();
  int numberOfOutputs = this->m_emulator->getNumberOfOutputs();
  for (int i = 0; i < numberOfParameters; ++i) {
    // std::string name("param");
    // double min = 0.0;
    // double max = 1.0;
    std::string name(this->m_emulator->getParameterName(i));
    double min = this->m_emulator->getParameterMinimum(i);
    double max = this->m_emulator->getParameterMaximum(i);
    this->AddParameter(name, min, max);
  }
  for (int i = 0; i < numberOfOutputs; ++i) {
    // std::string name("output");
    std::string name(this->m_emulator->getOutputName(i));
    this->AddScalarOutputName(name );
  }
  return Model::NO_ERROR;
}

/**
 * Get the scalar outputs from the model evaluated at x.  If an
 * error happens, the scalar output array will be left incomplete.
 */
Model::ErrorType GaussianProcessEmulatedModel::GetScalarOutputsAndCovariance(
  const std::vector< double > & parameters,
  std::vector< double > & scalars,
  std::vector< double > & scalarCovariance) const
{
  if (this->m_emulator == NULL)
    return Model::OTHER_ERROR;

  emulator * emu = const_cast<emulator * >( this->m_emulator );

  if (this->GetNumberOfParameters() != parameters.size())
    return Model::OTHER_ERROR;

  emu->QueryEmulator(parameters, scalars, scalarCovariance);
  return Model::NO_ERROR;
}

/**
 * Get the scalar outputs from the model evaluated at a position in
 * the parameter space.
 */
Model::ErrorType GaussianProcessEmulatedModel::GetScalarOutputs(
  const std::vector< double > & parameters,
  std::vector< double > & scalars ) const
{
  std::vector< double > scalarCovariance;
  return this->GetScalarOutputsAndCovariance(
    parameters, scalars, scalarCovariance);
}

}; // namespace madai




