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

#include "Model.h"

namespace madai {

Model
::Model() :
  m_GradientEstimateStepSize( 1.0e-4 ),
  m_StateFlag( UNINITIALIZED )
{
}


Model
::~Model()
{
}


bool
Model
::IsReady() const
{
  return ( m_StateFlag == READY );
}


unsigned int
Model
::GetNumberOfParameters() const
{
  return static_cast<unsigned int>(m_Parameters.size());
}


const std::vector< Parameter > &
Model
::GetParameters() const
{
  return m_Parameters;
}


unsigned int
Model
::GetNumberOfScalarOutputs() const
{
  return static_cast<unsigned int>(m_ScalarOutputNames.size());
}


Model::ErrorType
Model
::GetRange( unsigned int parameterIndex, double range[2] ) const
{
  if ( parameterIndex > this->GetNumberOfParameters() ) {
    return INVALID_PARAMETER_INDEX;
  }

  range[0] = this->m_Parameters.at(parameterIndex).m_MinimumPossibleValue;
  range[1] = this->m_Parameters.at(parameterIndex).m_MaximumPossibleValue;

  return NO_ERROR;
}


const std::vector< std::string > &
Model
::GetScalarOutputNames() const
{
  return m_ScalarOutputNames;
}


Model::ErrorType
Model
::GetScalarAndGradientOutputs(
  const std::vector< double > & parameters,
  const std::vector< bool > & activeParameters,
  std::vector< double > & scalars,
  unsigned int outputIndex,
  std::vector< double > & gradient) const
{
  if ( outputIndex >= this->GetNumberOfScalarOutputs() ) {
    return INVALID_OUTPUT_INDEX;
  }

  if ( static_cast< unsigned int >( activeParameters.size() ) !=
       this->GetNumberOfParameters() ) {
    return INVALID_ACTIVE_PARAMETERS;
  }

  // Make a copy of the parameters that we can work with
  std::vector< double > parametersCopy( parameters );

  // Clear the output vectors
  scalars.clear();
  gradient.clear();

  ErrorType scalarOutputError;

  double h = m_GradientEstimateStepSize;
  for ( unsigned int i = 0; i < this->GetNumberOfParameters(); ++i ) {

    if ( activeParameters[i] ) {
      // Save the original parameter value
      double originalParameterValue = parametersCopy[i];

      // Compute the scalar outputs for a forward step
      parametersCopy[i] = parameters[i] + h;
      std::vector< double > forwardScalars;
      scalarOutputError = this->GetScalarOutputs( parametersCopy, forwardScalars );
      if ( scalarOutputError != NO_ERROR ) {
        return scalarOutputError;
      }

      // Compute the scalar outputs for a backward step
      parametersCopy[i] = parameters[i] - h;
      std::vector< double > backwardScalars;
      scalarOutputError = this->GetScalarOutputs( parametersCopy, backwardScalars );
      if ( scalarOutputError != NO_ERROR ) {
        return scalarOutputError;
      }

      // Compute the partial derivative with central differences
      double f = forwardScalars[ outputIndex ];
      double b = backwardScalars[ outputIndex ];
      double partialDerivative = ( f - b ) / ( 2.0 * h );

      // Store the partial derivative in the gradient output
      gradient.push_back( partialDerivative );

      // Restore the original parameter value
      parametersCopy[i] = originalParameterValue;
    }

  }

  // Now compute the scalars
  scalarOutputError = this->GetScalarOutputs( parameters, scalars );
  if ( scalarOutputError != NO_ERROR ) {
    return scalarOutputError;
  }

  return NO_ERROR;
}


void
Model
::SetGradientEstimateStepSize( double stepSize )
{
  m_GradientEstimateStepSize = stepSize;
}


double
Model
::GetGradientEstimateStepSize() const
{
  return m_GradientEstimateStepSize;
}


std::string
Model
::GetErrorTypeAsString( ErrorType error )
{
  std::string outputString( "NO_ERROR" );

  switch ( error ) {

  case INVALID_PARAMETER_INDEX:
    outputString = std::string( "INVALID_PARAMETER_INDEX" );
    break;

  case INVALID_OUTPUT_INDEX:
    outputString = std::string( "INVALID_OUTPUT_INDEX" );
    break;

  case INVALID_ACTIVE_PARAMETERS:
    outputString = std::string( "INVALID_ACTIVE_PARAMETERS" );
    break;

  case FILE_NOT_FOUND_ERROR:
    outputString = std::string( "FILE_NOT_FOUND_ERROR" );
    break;

  case METHOD_NOT_IMPLEMENTED:
    outputString = std::string( "METHOD_NOT_IMPLEMENTED" );
    break;

  case OTHER_ERROR:
    outputString = std::string( "OTHER_ERROR" );
    break;

  default:
    break;

  }

  return outputString;
}


void
Model
::AddParameter( const std::string & name,
                double minimumPossibleValue,
                double maximumPossibleValue )
{
  m_Parameters.push_back( Parameter(name, minimumPossibleValue, maximumPossibleValue) );
}


void
Model
::AddScalarOutputName( const std::string & name )
{
  m_ScalarOutputNames.push_back( name );
}


} // end namespace madai
