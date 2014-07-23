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

#include <algorithm> // std::count
#include <cassert>

#include "Sampler.h"
#include "Parameter.h"

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
::SetModel( const Model * model )
{
  if (model != m_Model)
    this->Initialize( model );
  // no need to reinitialize if model already there..
}


const Model *
Sampler
::GetModel() const
{
  return m_Model;
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


std::set< std::string >
Sampler
::GetActiveParameters() const
{
  return m_ActiveParameters;
}


const std::vector< bool > &
Sampler
::GetActiveParametersByIndex() const
{
  return m_ActiveParameterIndices;
}


unsigned int
Sampler
::GetNumberOfActiveParameters() const
{
  return static_cast< unsigned int >( m_ActiveParameters.size() );
}


bool
Sampler
::IsParameterActive( const std::string & parameterName ) const
{
  return ( m_ActiveParameters.find( parameterName ) !=
           m_ActiveParameters.end() );
}


bool
Sampler
::IsParameterActive( unsigned int parameterIndex ) const
{
  assert(parameterIndex < m_ActiveParameterIndices.size());
  assert(this->GetNumberOfParameters() == this->GetParameters().size());
  return m_ActiveParameterIndices[parameterIndex];
}


void
Sampler
::ActivateParameter( const std::string & parameterName )
{
  unsigned int parameterIndex = this->GetParameterIndex( parameterName );
  assert( parameterIndex != static_cast< unsigned int >(-1) );
  // should return an error, but this is a void function :(
  if ( parameterIndex != static_cast< unsigned int >(-1) ) {
    m_ActiveParameterIndices[parameterIndex] = true;
    m_ActiveParameters.insert( parameterName );
  }
  assert (std::count(m_ActiveParameterIndices.begin(),
                     m_ActiveParameterIndices.end(), true)
          == static_cast< int >(m_ActiveParameters.size()));
}


void
Sampler
::ActivateParameter( unsigned int parameterIndex ) {
  assert(parameterIndex < this->GetNumberOfParameters());
  assert(this->GetNumberOfParameters() == this->GetParameters().size());
  assert(this->GetNumberOfParameters() == m_ActiveParameterIndices.size());
  m_ActiveParameterIndices[parameterIndex] = true;
  m_ActiveParameters.insert(this->GetParameters()[parameterIndex].m_Name);
  assert (std::count(m_ActiveParameterIndices.begin(),
                     m_ActiveParameterIndices.end(), true)
          == static_cast< int >(m_ActiveParameters.size()));
}


void
Sampler
::DeactivateParameter( const std::string & parameterName )
{
  unsigned int parameterIndex = this->GetParameterIndex( parameterName );
  assert ( parameterIndex != static_cast< unsigned int >(-1) );
     // should return an error, but this is a void function :(
  if ( parameterIndex != static_cast< unsigned int >(-1) ) {
    m_ActiveParameterIndices[ parameterIndex ] = false;
    m_ActiveParameters.erase( parameterName );
  }
  assert (std::count(m_ActiveParameterIndices.begin(),
                     m_ActiveParameterIndices.end(), true)
          == static_cast< int >(m_ActiveParameters.size()));
}


void
Sampler
::DeactivateParameter( unsigned int parameterIndex ) {
  assert(parameterIndex < this->GetNumberOfParameters());
  assert(this->GetNumberOfParameters() == this->GetParameters().size());
  assert(this->GetNumberOfParameters() == m_ActiveParameterIndices.size());
  m_ActiveParameterIndices[parameterIndex] = false;
  m_ActiveParameters.erase(this->GetParameters()[parameterIndex].m_Name);
  assert (std::count(m_ActiveParameterIndices.begin(),
                     m_ActiveParameterIndices.end(), true)
          == static_cast< int >(m_ActiveParameters.size()));
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
  if (m_CurrentParameters[parameterIndex] != value) {
    m_CurrentParameters[parameterIndex] = value;
    this->ParameterSetExternally(); // set dirty flag
  }
  return NO_ERROR;
}


Sampler::ErrorType
Sampler
::SetParameterValue( unsigned int parameterIndex, double value )
{
  if ( parameterIndex >= this->GetNumberOfParameters() )
    {
    return INVALID_PARAMETER_INDEX_ERROR;
    }
  if (m_CurrentParameters[parameterIndex] != value) {
    m_CurrentParameters[parameterIndex] = value;
    this->ParameterSetExternally(); // set dirty flag
  }
  return NO_ERROR;
}


Sampler::ErrorType
Sampler
::SetParameterValues(const std::vector< double > & parameterValues)
{
  if ( parameterValues.size() != this->GetNumberOfParameters() ) {
    // wrong vector size
    return INVALID_PARAMETER_INDEX_ERROR;
  }
  m_CurrentParameters = parameterValues;
  this->ParameterSetExternally(); // set dirty flag
  return NO_ERROR;
}


double
Sampler
::GetParameterValue( const std::string & parameterName ) const
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


const std::vector< double > &
Sampler
::GetCurrentOutputs() const
{
  return m_CurrentOutputs;
}


double
Sampler
::GetCurrentLogLikelihood() const
{
  return m_CurrentLogLikelihood;
}


const std::vector< double > &
Sampler
::GetCurrentLogLikelihoodValueGradient() const
{
  return m_CurrentLogLikelihoodValueGradient;
}

const std::vector< double > &
Sampler
::GetCurrentLogLikelihoodErrorGradient() const
{
  return m_CurrentLogLikelihoodErrorGradient;
}


std::string
Sampler
::GetErrorTypeAsString( ErrorType error )
{
  switch(error) {
  case NO_ERROR:
    return "NO_ERROR";
  case INVALID_PARAMETER_INDEX_ERROR:
    return "INVALID_PARAMETER_INDEX_ERROR";
  default:
    return "UNKNOWN";
  }
}


void
Sampler
::Initialize( const Model * model )
{
  assert(model != NULL);
  if (! model)
    return;

  m_Model = model;
  unsigned int np = m_Model->GetNumberOfParameters();

  // Activate all parameters by default.
  const std::vector< Parameter > & params = m_Model->GetParameters();
  assert(np == params.size());

  m_ActiveParameters.clear();
  for (unsigned int i = 0; (i < np); ++i) {
    m_ActiveParameters.insert( params[i].m_Name );
  }
  m_ActiveParameterIndices =
    std::vector< bool >( model->GetNumberOfParameters(), true );

  m_CurrentParameters =
    std::vector< double >( model->GetNumberOfParameters(), 0.0 );
  m_CurrentOutputs =
    std::vector< double >( model->GetNumberOfScalarOutputs(), 0.0 );
  for (unsigned int i = 0; (i < np); ++i) {
    m_CurrentParameters[i] =
      params[i].GetPriorDistribution()->GetPercentile(0.5);
    // set a reasonable default value for the parameters.
    // most subclasses will do something smarter.
  }
  this->ParameterSetExternally(); // valid values for m_CurrentOutputs
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


void
Sampler
::ParameterSetExternally()
{
  if (! m_Model)
    return;
  if (m_CurrentParameters.size() != m_Model->GetNumberOfParameters())
    return;
  if (m_CurrentOutputs.size() == m_Model->GetNumberOfScalarOutputs())
    return;
  Model::ErrorType error = m_Model->GetScalarOutputsAndLogLikelihood(
      m_CurrentParameters, m_CurrentOutputs, m_CurrentLogLikelihood);
  assert(error == Model::NO_ERROR);
  (void) error;
}


} // end namespace madai
