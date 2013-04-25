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

#include "Gaussian2DModel.h"

#include <cmath>

#include "UniformDistribution.h"


namespace madai {

Gaussian2DModel
::Gaussian2DModel() :
  m_MeanX( 23.2 ),
  m_MeanY( -14.0 ),
  m_StandardDeviationX( 4.0 ),
  m_StandardDeviationY( 12.3 )
{
  UniformDistribution xPrior;
  xPrior.SetMinimum( m_MeanX - 10.0 * m_StandardDeviationX );
  xPrior.SetMaximum( m_MeanX + 10.0 * m_StandardDeviationX );
  this->AddParameter( "X",  xPrior );

  UniformDistribution yPrior;
  yPrior.SetMinimum( m_MeanY - 10.0 * m_StandardDeviationY );
  yPrior.SetMaximum( m_MeanY + 10.0 * m_StandardDeviationY );
  this->AddParameter( "Y", yPrior );

  this->AddScalarOutputName( "Value" );

  // Usually the observed scalar values would be set from outside the
  // Model object. As this is a test object, however, we are setting
  // the observed values here.
  m_ObservedScalarValues.push_back( 1.0 );
  m_ObservedScalarCovariance.push_back( 0.1 );

  // This model is always ready
  m_StateFlag = Model::READY;
}


Model::ErrorType
Gaussian2DModel
::LoadConfigurationFile( const std::string fileName )
{
  // Member variables should be set from values ready from a file.

  return Model::NO_ERROR;
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
  assert( activeParameters.size() == 2 );
  assert( m_ObservedScalarValues.size() == 1 );
  if ( activeParameters[0] ) {
    gradient.push_back( -(functionValue - m_ObservedScalarValues[0]) *
                        this->PartialX( parameters[0], functionValue ) /
                        m_ObservedScalarCovariance[0] );
  }
  if ( activeParameters[1] ) {
    gradient.push_back( -(functionValue - m_ObservedScalarValues[0]) *
                        this->PartialY( parameters[1], functionValue ) /
                        m_ObservedScalarCovariance[0] );
  }

  return NO_ERROR;
}


Model::ErrorType
Gaussian2DModel
::SetObservedScalarValues( const std::vector< double > & observedScalarValues )
{
  // Do nothing. The values are set internally.
  return NO_ERROR;
}


Model::ErrorType
Gaussian2DModel
::SetObservedScalarCovariance( const std::vector< double > & observedScalarCovariance)
{
  // Do nothing. The values are set internally.
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
::SetMeans( double means[2] )
{
  m_MeanX = means[0];
  m_MeanY = means[1];
}

void
Gaussian2DModel
::GetMeans( double & MeanX, double & MeanY ) const
{
  MeanX = this->m_MeanX;
  MeanY = this->m_MeanY;
}


void
Gaussian2DModel
::SetDeviations( double stddev[2] )
{
  m_StandardDeviationX = stddev[0];
  m_StandardDeviationY = stddev[1];
}


void
Gaussian2DModel
::GetDeviations( double & DevX, double & DevY ) const
{
  DevX = this->m_StandardDeviationX;
  DevY = this->m_StandardDeviationY;
}


} // end namespace madai
