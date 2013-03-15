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
  if (! m_GPME.Load(input))
    return Model::OTHER_ERROR;
  if (m_GPME.m_Status != GaussianProcessEmulator::READY)
    return Model::OTHER_ERROR;
  m_StateFlag = READY;
  // std::vector<T>::operator=(const std::vector<T>&)
  m_Parameters = m_GPME.m_Parameters;
  m_ScalarOutputNames = m_GPME.m_OutputNames;
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
