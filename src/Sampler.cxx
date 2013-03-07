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

#include "Sampler.h"
#include "Parameter.h"
#include <cassert>


namespace madai {


Sampler
::Sampler() :
  m_Model( NULL )
{
}


Sampler
::~Sampler()
{
}


void
Sampler
::Initialize( const Model * model )
{
  assert(model != NULL);

  m_Model = model;
  unsigned int np = m_Model->GetNumberOfParameters();

  // Activate all parameters by default.
  const std::vector< Parameter > & params = m_Model->GetParameters();
  assert(np == params.size());

  m_ActiveParameters.clear();
  for (unsigned int i = 0; (i < np); ++i) {
    m_ActiveParameters.insert( params[i].m_Name );
  }

  m_CurrentParameters = std::vector< double >( model->GetNumberOfParameters(), 0.0 );
  m_ActiveParameterIndices = std::vector< bool >( model->GetNumberOfParameters(), true );
}


void
Sampler
::SetModel( const Model * model )
{
  this->Initialize( model );
}


const Model *
Sampler
::GetModel() const
{
  return m_Model;
}


std::set< std::string >
Sampler
::GetActiveParameters()
{
  return m_ActiveParameters;
}


void
Sampler
::ActivateParameter( const std::string & parameterName )
{
  bool found = false;
  unsigned int parameterIndex = this->GetParameterIndex( parameterName );
  if ( parameterIndex != static_cast< unsigned int >(-1) ) {
    m_ActiveParameterIndices[parameterIndex] = true;
    m_ActiveParameters.insert( parameterName );
    found = true;
  }

  assert(found); // should return an error, but this is a void function :(
}


void
Sampler
::ActivateParameter( unsigned int parameterIndex ) {
  assert(parameterIndex < this->GetNumberOfParameters());
  assert(this->GetNumberOfParameters() == this->GetParameters().size());
  assert(this->GetNumberOfParameters() == m_ActiveParameterIndices.size());
  m_ActiveParameterIndices[parameterIndex] = true;
  m_ActiveParameters.insert(this->GetParameters()[parameterIndex].m_Name);
}


void
Sampler
::DeactivateParameter( const std::string & parameterName )
{
  bool found = false;
  unsigned int parameterIndex = this->GetParameterIndex( parameterName );
  if ( parameterIndex != static_cast< unsigned int >(-1) ) {
    m_ActiveParameterIndices[ parameterIndex ] = false;
    m_ActiveParameters.erase( parameterName );
    found = true;
  }

  assert(found); // should return an error, but this is a void function :(
}


void
Sampler
::DeactivateParameter( unsigned int parameterIndex ) {
  assert(parameterIndex < this->GetNumberOfParameters());
  assert(this->GetNumberOfParameters() == this->GetParameters().size());
  assert(this->GetNumberOfParameters() == m_ActiveParameterIndices.size());
  m_ActiveParameterIndices[parameterIndex] = false;
  m_ActiveParameters.erase(this->GetParameters()[parameterIndex].m_Name);
}


unsigned int
Sampler
::GetNumberOfParameters() const {
  if (this->GetModel() != NULL)
    return this->GetModel()->GetNumberOfParameters();
  else
    return 0;
}


const std::vector< Parameter > &
Sampler
::GetParameters() const {
  assert (this->GetModel() != NULL);
  return this->GetModel()->GetParameters();
}


unsigned int
Sampler
::GetNumberOfActiveParameters() const
{
  return static_cast< unsigned int >( m_ActiveParameters.size() );
}


bool
Sampler
::IsParameterActive( const std::string & parameterName )
{
  return ( m_ActiveParameters.find( parameterName ) !=
           m_ActiveParameters.end() );
}


Sampler::ErrorType
Sampler
::SetParameterValue( const std::string & parameterName, double value )
{
  unsigned int parameterIndex = this->GetParameterIndex( parameterName );

  if ( parameterIndex == static_cast< unsigned int >( -1 ) )
    {
    // Error. Parameter not found.
    return INVALID_PARAMETER_INDEX_ERROR;
    }

  m_CurrentParameters[parameterIndex] = value;

  // TODO - set dirty flag

  return NO_ERROR;
}


double
Sampler
::GetParameterValue( const std::string & parameterName )
{
  unsigned int parameterIndex = this->GetParameterIndex( parameterName );

  if ( parameterIndex == static_cast< unsigned int >( -1 ) ) {
    return 0.0;
  }

  return m_CurrentParameters[parameterIndex];
}




const std::vector< double > &
Sampler
::GetCurrentParameters() const
{
  return m_CurrentParameters;
}




unsigned int
Sampler
::GetParameterIndex( const std::string & parameterName ) const
{
  const std::vector< Parameter > & parameters = this->m_Model->GetParameters();
  for ( unsigned int i = 0; i < m_Model->GetNumberOfParameters(); i++ )
    {
    if ( parameters[i].m_Name == parameterName )
      return i;
    }

  return static_cast< unsigned int >(-1); // Intentional underflow
}


} // end namespace madai

