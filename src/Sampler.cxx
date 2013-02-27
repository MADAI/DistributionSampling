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
::Sampler( const Model *model ) :
  m_Model(model),
  m_CurrentParameters(model->GetNumberOfParameters(),0.0),
  m_OutputScalarToOptimizeIndex( 0 ),
  m_ActiveParameterIndices(model->GetNumberOfParameters(), true),
  m_OptimizeOnLikelihood(false)
{
  assert(model != NULL);
  unsigned int np = this->m_Model->GetNumberOfParameters();
  unsigned int nt = this->m_Model->GetNumberOfScalarOutputs();
  assert(this->m_OutputScalarToOptimizeIndex < nt);

  // Initialize the vector for current parameters.
  m_CurrentParameters.resize( m_Model->GetNumberOfParameters() );

  // Activate all parameters by default.
  const std::vector< Parameter > & params = this->m_Model->GetParameters();
  assert(np == params.size());

  for (unsigned int i = 0; (i < np); ++i)
    {
    this->m_ActiveParameters.insert( params[i].m_Name );
    }

  this->m_OutputScalarToOptimize
    = this->m_Model->GetScalarOutputNames().at(
      this->m_OutputScalarToOptimizeIndex);
}


Sampler
::~Sampler()
{
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


Sampler::ErrorType
Sampler
::SetOutputScalarToOptimize( const std::string & scalarName )
{
  unsigned int idx = this->GetOutputScalarIndex( scalarName );
  if ( idx == static_cast< unsigned int >( -1 ) )
    return INVALID_PARAMETER_INDEX_ERROR;
  this->m_OutputScalarToOptimize = scalarName;
  this->m_OutputScalarToOptimizeIndex = idx;
  return NO_ERROR;
}


Sampler::ErrorType
Sampler
::SetOutputScalarToOptimize( unsigned int idx )
{
  if (idx >= m_Model->GetNumberOfScalarOutputs())
    return INVALID_PARAMETER_INDEX_ERROR;
  this->m_OutputScalarToOptimizeIndex = idx;
  this->m_OutputScalarToOptimize = this->m_Model->GetScalarOutputNames()[idx];
  return NO_ERROR;
}


std::string
Sampler
::GetOutputScalarToOptimizeName()
{
  return this->m_OutputScalarToOptimize;
}


unsigned int
Sampler
::GetOutputScalarToOptimizeIndex() const
{
  return this->m_OutputScalarToOptimizeIndex;
}


const std::vector< double > &
Sampler
::GetCurrentParameters() const
{
  return m_CurrentParameters;
}


unsigned int
Sampler
::GetOutputScalarIndex( const std::string & scalarName ) const
{
  std::vector< std::string > const & outputs = this->m_Model->GetScalarOutputNames();
  for ( unsigned int i = 0; i < outputs.size(); i++ )
    {
    if ( outputs[i] == scalarName )
      return i;
    }

  return static_cast< unsigned int >(-1); // Intentional underflow
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


// To check if the model supplis a method for calculating the Likelihood and Prior
bool
Sampler
::IsLikeAndPrior() const
{
  std::cerr << "Using point at center of each parameter range" << std::endl;
  double * range = new double[2]();
  std::vector< double > temp_vals( this->m_Model->GetNumberOfParameters(), 0.0 );

  for ( unsigned int i = 0; i < this->m_Model->GetNumberOfParameters(); i++ ) {
    this->m_Model->GetRange( i, range );
    temp_vals[i] = (range[0] + range[1]) / 2;
    std::cerr << temp_vals[i] << "  ";
  }
  std::cerr << std::endl;

  double Likelihood, Prior;
  if ( this->m_Model->GetLikeAndPrior( temp_vals, Likelihood, Prior ) != Model::NO_ERROR ) {
    std::cerr << "GetLikeAndPrior not defined in model" << std::endl;
    return false;
  } else {
    std::cerr << "GetLikeAndPrior is defined" << std::endl;
    return true;
  }
}



bool
Sampler
::GetOptimizeOnLikelihood() const {
  return this->m_OptimizeOnLikelihood;
}

void
Sampler
::SetOptimizeOnLikelihood(bool val) {
  this->m_OptimizeOnLikelihood = val;
}

} // end namespace madai

