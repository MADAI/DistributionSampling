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

#include "LangevinSampler.h"

#include <cassert>
#include <algorithm>
#include <cmath>

namespace madai {


std::vector< double >
LangevinSampler
::GetGradient( const std::vector< double > Parameters, const Model * m) 
{
  std::vector< double > Gradient;
  m->GetScalarAndGradientOutputs(
    Parameters, m_ActiveParameterIndices,
    m_CurrentOutputs, Gradient );
  double gradient_size = 0;
  for ( unsigned int i = 0; i < Gradient.size(); i++ ) {
    Gradient[i] *= ( m_UpperLimit[i] - m_LowerLimit[i] ); // Scale gradient
    gradient_size += Gradient[i] * Gradient[i]; // Determine size of gradient
  }
  gradient_size = std::sqrt( gradient_size );
  if ( gradient_size > m_LargestGradient ) { // Check to see if updating largest gradient and step size is necessary
    m_LargestGradient = gradient_size;
    m_StepSize = 1.0 / ( 10.0 * m_LargestGradient );
  }
  if ( m_NumberOfElementsInAverage < (Parameters.size() * 2000) ) {
    m_AverageGradient = ( double( m_NumberOfElementsInAverage ) * m_AverageGradient + gradient_size )
                        / double( m_NumberOfElementsInAverage + 1 );
    m_GaussianWidth = m_LargestGradient;
    m_NumberOfElementsInAverage++;
    // Reset gradient in order to sample space randomly ( no weighting )
    for ( unsigned int i = 0; i < Gradient.size(); i++ ) {
      Gradient[i] = 0;
    }
  } else if ( m_NumberOfElementsInAverage == (Parameters.size() * 2000) ) {
    m_GaussianWidth = 2.0 * m_AverageGradient;
  }
  
  return Gradient;
}


LangevinSampler
::LangevinSampler() :
  Sampler()
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

  m_UpperLimit.clear();
  m_LowerLimit.clear();
  size_t t = m_Model->GetNumberOfParameters();
  // Random initial starting point
  const std::vector< Parameter > & params = m_Model->GetParameters();
  for ( unsigned int i = 0; i < t; i++ ) {
    const Distribution * priorDist = params[i].GetPriorDistribution();
    m_CurrentParameters[i] = priorDist->GetSample(m_Random);
    double upper = priorDist->GetPercentile( 0.9 );
    double lower = priorDist->GetPercentile( 0.1 );
    m_LowerLimit.push_back( lower - ( upper - lower ) / 8.0 );
    m_UpperLimit.push_back( upper + ( upper - lower ) / 8.0 );
  }
  m_StepSize = 0.1;
  m_LargestGradient = 1.0;
  m_GaussianWidth = 1.0;
  m_AverageGradient = 0;
  m_NumberOfElementsInAverage = 0;
}


Sample
LangevinSampler
::NextSample()
{
  Model * m = const_cast< Model * >(m_Model);
  
  assert( static_cast<unsigned int> (
              std::count( m_ActiveParameterIndices.begin(),
                          m_ActiveParameterIndices.end(), true ))
          == this->GetNumberOfActiveParameters());
          
  // Get the gradient of the log likelihood at the current point
  std::vector< double > CurrentGradient = this->GetGradient( m_CurrentParameters, m );
  
  // Scale parameters by parameterspace
  for ( unsigned int i = 0; i < m_CurrentParameters.size(); i++ ) {
    m_CurrentParameters[i] = ( m_CurrentParameters[i] - m_LowerLimit[i] ) / ( m_UpperLimit[i] - m_LowerLimit[i] );
  }
  
  // Take a half step
  std::vector< double > NewParameters = m_CurrentParameters;
  for ( unsigned int i = 0; i < NewParameters.size(); i++ ) {
    NewParameters[i] += m_StepSize * ( CurrentGradient[i] + m_Random.Gaussian( 0., m_GaussianWidth ) ) / 2.0;
    NewParameters[i] = NewParameters[i] * ( m_UpperLimit[i] - m_LowerLimit[i] ) + m_LowerLimit[i];
  }
  // Get Gradient of LL at the new point
  std::vector< double > NewGradient = this->GetGradient( NewParameters, m );
  
  // Take final step and scale back to original units
  for ( unsigned int i = 0; i < m_CurrentParameters.size(); i++ ) {
    m_CurrentParameters[i] += m_StepSize * ( NewGradient[i] + m_Random.Gaussian( 0., m_GaussianWidth ) );
    m_CurrentParameters[i] = m_CurrentParameters[i] * ( m_UpperLimit[i] - m_LowerLimit[i] ) + m_LowerLimit[i];
  }
  
  // Reflect if past the upper and lower limits
  for ( unsigned int i = 0; i < m_CurrentParameters.size(); i++ ) {
    if ( m_CurrentParameters[i] < m_LowerLimit[i] ) {
      m_CurrentParameters[i] = m_LowerLimit[i] + ( m_LowerLimit[i] - m_CurrentParameters[i] );
    } else if ( m_CurrentParameters[i] > m_UpperLimit[i] ) {
      m_CurrentParameters[i] = m_UpperLimit[i] - ( m_CurrentParameters[i] - m_UpperLimit[i] );
    }
  }
  
  // Get loglikelihood at the new parameters
  double LogLikelihood;
  m->GetScalarOutputsAndLogLikelihood(
    m_CurrentParameters, m_CurrentOutputs, LogLikelihood );
    
  // Check for NaN
  assert( LogLikelihood == LogLikelihood );
  
  return Sample( m_CurrentParameters,
                 m_CurrentOutputs,
                 LogLikelihood );
}

} // end namespace madai
