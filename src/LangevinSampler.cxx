/*=========================================================================
 *
 *  Copyright The University of North Carolina at Chapel Hill
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

#include "LangevinSampler.h"

#include <cassert>
#include <algorithm>
#include <cmath>

namespace madai {


LangevinSampler
::LangevinSampler() :
  Sampler(),
  m_TimeStep( 1.0e-2 ),
  m_MeanTime( 1 ),
  m_KickStrength( 1 ),
  m_DragCoefficient( 1.0e-2 ),
  m_MassScale( 1 )
{
}


LangevinSampler
::~LangevinSampler()
{
}


void
LangevinSampler
::Initialize( const Model * model )
{
  assert( model != NULL );
  m_Model = model;
  
  Sampler::Initialize( model );
  
  // Random initial starting point
  const std::vector< Parameter > & params = m_Model->GetParameters();
  for ( unsigned int i = 0; i < m_Model->GetNumberOfParameters(); i++ ) {
    const Distribution * priorDist = params[i].GetPriorDistribution();
    m_CurrentParameters[i] = priorDist->GetSample(m_Random);
    m_CurrentVelocities.push_back(0.0);
  }
  
  m_TimeBeforeNextKick = m_Random.Gaussian( m_MeanTime, m_MeanTime / 10.0 );
}


Sample
LangevinSampler
::NextSample()
{
  Model * m = const_cast< Model * >(m_Model);
  double TimeLeft = m_TimeStep;
   
  unsigned int numberOfActiveParameters = this->GetNumberOfActiveParameters();
   
  assert( std::count( m_ActiveParameterIndices.begin(),
                      m_ActiveParameterIndices.end(), true )
          == numberOfActiveParameters);
   
  // Get the gradient of the Log Likelihood at the current point
  std::vector< double > Gradient;
  m->GetScalarAndGradientOutputs(
    m_CurrentParameters, m_ActiveParameterIndices,
    m_CurrentOutputs, Gradient );
    
  // Get loglikelihood at the current parameters
  double LogLikelihood;
  m->GetScalarOutputsAndLogLikelihood(
    m_CurrentParameters, m_CurrentOutputs, LogLikelihood );
  // Check for NaN
  assert( LogLikelihood == LogLikelihood );
  
  std::vector< double > Accel = Gradient;
  for ( unsigned int i = 0; i < Accel.size(); i++ ) {
    Accel[i] -= m_DragCoefficient * m_CurrentVelocities[i];
    Accel[i] /= m_MassScale;
  }
  if ( m_TimeBeforeNextKick < m_TimeStep ) { // Check if kick happens
    // Take step up to m_TimeBeforeNextKick
    for ( unsigned int i = 0; i < m_CurrentParameters.size(); i++ ) {
      m_CurrentParameters[i] += m_CurrentVelocities[i] * m_TimeBeforeNextKick;
      m_CurrentParameters[i] += 0.5 * Accel[i] * m_TimeBeforeNextKick * m_TimeBeforeNextKick;
      m_CurrentVelocities[i] += Accel[i] * m_TimeBeforeNextKick;
    }
    // Get m_CurrentVelocities.size() number of random numbers
    double r[m_CurrentVelocities.size()];
    double temp = 0;
    for ( unsigned int i = 0; i < m_CurrentVelocities.size(); i++ ) {
      r[i] = m_Random.Uniform( -1, 1 );
      temp += r[i] * r[i];
    }
    temp = sqrt( temp );
    temp /= m_KickStrength;
    for ( unsigned int i = 0; i < m_CurrentVelocities.size(); i++ ) {
     m_CurrentVelocities[i] += r[i] / temp;
     Accel[i] -= m_DragCoefficient * r[i] / temp;
    }
    TimeLeft -= m_TimeBeforeNextKick;
    m_TimeBeforeNextKick += m_Random.Gaussian( m_MeanTime, m_MeanTime / 10.0 );
  }
  
  for ( unsigned int i = 0; i < m_CurrentParameters.size(); i++ ) {
    m_CurrentParameters[i] += m_CurrentVelocities[i] * TimeLeft;
    m_CurrentParameters[i] += 0.5 * Accel[i] * TimeLeft * TimeLeft;
    m_CurrentVelocities[i] += Accel[i] * TimeLeft;
  }
  
  m_TimeBeforeNextKick -= m_TimeStep;
  
  return Sample( m_CurrentParameters,
                 m_CurrentOutputs,
                 LogLikelihood );

}
  
  
void LangevinSampler
::SetTimeStep( double TimeStep )
{
  m_TimeStep = TimeStep;
}


void LangevinSampler
::SetKickStrength( double KickStrength )
{
  m_KickStrength = KickStrength;
}


void LangevinSampler
::SetMeanTimeBetweenKicks( double MeanTime )
{
  m_MeanTime = MeanTime;
}


void LangevinSampler
::SetDragCoefficient( double DragCoefficient )
{
  m_DragCoefficient = DragCoefficient;
}
  

void LangevinSampler
::SetMassScale( double MassScale )
{
  m_MassScale = MassScale;
}

Sampler::ErrorType
LangevinSampler
::SetVelocity( const std::string & parameterName, double Velocity )
{
  unsigned int parameterIndex = this->GetParameterIndex( parameterName );
  
  if ( parameterIndex == static_cast< unsigned int >( -1 ) ) {
    return INVALID_PARAMETER_INDEX_ERROR;
  }
  
  m_CurrentVelocities[parameterIndex] = Velocity;
  
  return NO_ERROR;
}
  
} // end namespace madai
