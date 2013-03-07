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

#include "Gaussian2DModel.h"

#include <cmath>


namespace madai {

Gaussian2DModel
::Gaussian2DModel() :
  m_MeanX( 23.2 ),
  m_MeanY( -14.0 ),
  m_StandardDeviationX( 4.0 ),
  m_StandardDeviationY( 12.3 )
{
  this->AddParameter( "X" );
  this->AddParameter( "Y" );

  this->AddScalarOutputName( "Value" );
}


Model::ErrorType
Gaussian2DModel
::LoadConfigurationFile( const std::string fileName )
{
  // Member variables should be set from values ready from a file.

  return Model::NO_ERROR;
}

void
Gaussian2DModel
::GetRange( unsigned int parameterIndex, double range[2] )
{
  range[0] = m_Parameters[parameterIndex].m_MinimumPossibleValue;
  range[1] = m_Parameters[parameterIndex].m_MaximumPossibleValue;
}


Model::ErrorType
Gaussian2DModel
::GetScalarOutputs( const std::vector< double > & parameters,
                    std::vector< double > & scalars ) const
{
  scalars.clear(); // Remove all elements from the output vector.

  // Compute "Value" output.
  double x = parameters[0];
  double y = parameters[1];

  double dx = x - m_MeanX;
  double dy = y - m_MeanY;
  double sx = m_StandardDeviationX;
  double sy = m_StandardDeviationY;

  // Negate the Gaussian so there is a well-defined global minimum
  double value = exp( -( ((dx*dx) / (2.0 * sx * sx)) +
                         ((dy*dy) / (2.0 * sy * sy)) ) );

  scalars.push_back( value );

  return NO_ERROR;
}


Model::ErrorType
Gaussian2DModel
::GetScalarAndGradientOutputs( const std::vector< double > & parameters,
                               const std::vector< bool > & activeParameters,
                               std::vector< double > & scalars,
                               std::vector< double > & gradient) const
{
  scalars.clear();
  gradient.clear();

  ErrorType error = this->GetScalarOutputs( parameters, scalars );
  if ( error != NO_ERROR ) {
    return error;
  }

  if ( activeParameters.size() != this->GetNumberOfParameters() ) {
    return INVALID_ACTIVE_PARAMETERS;
  }

  double functionValue = scalars[0];
  unsigned int activeParameter = 0;
  if ( activeParameters[0] ) {
    gradient.push_back( this->PartialX( parameters[0], functionValue ) );
  }
  if ( activeParameters[1] ) {
    gradient.push_back( this->PartialY( parameters[1], functionValue ) );
  }

  return NO_ERROR;
}


double
Gaussian2DModel
::PartialX( double x, double value ) const
{
  double dx = x - m_MeanX;
  double sx = m_StandardDeviationX;

  return -(value * dx) / (sx * sx);
}


double
Gaussian2DModel
::PartialY( double y, double value ) const
{
  double dy = y - m_MeanY;
  double sy = m_StandardDeviationY;

  return -(value * dy) / (sy * sy);
}


void
Gaussian2DModel
::GetDeviations( double & DevX, double & DevY ) const
{
  DevX = this->m_StandardDeviationX;
  DevY = this->m_StandardDeviationY;
}


void
Gaussian2DModel
::GetMeans( double & MeanX, double & MeanY ) const
{
  MeanX = this->m_MeanX;
  MeanY = this->m_MeanY;
}

} // end namespace madai
