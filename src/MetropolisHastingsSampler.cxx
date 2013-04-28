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

#include "MetropolisHastingsSampler.h"

#include <cassert> // assert
#include <cmath> // std::exp
#include <algorithm> // std::count

namespace madai {


MetropolisHastingsSampler
::MetropolisHastingsSampler() :
  Sampler(),
  m_StepSize( 1.0e-2 )
{
}


MetropolisHastingsSampler
::~MetropolisHastingsSampler()
{
}


void
MetropolisHastingsSampler
::Initialize( const Model * model )
{
  assert(model != NULL);
  m_Model = model;

  Sampler::Initialize( model );

  const std::vector< Parameter > & params = m_Model->GetParameters();
  m_StepScales.resize( model->GetNumberOfParameters() );
  for ( unsigned int i = 0; i < model->GetNumberOfParameters(); i++ ) {
    const Distribution * priorDist = params[i].GetPriorDistribution();
    m_CurrentParameters[i] = priorDist->GetSample(m_Random);
    // Random initial starting point
    m_StepScales[i]
      = priorDist->GetPercentile(0.75) - priorDist->GetPercentile(0.25);
    // set step scales
  }
  Model * m = const_cast< Model * >(m_Model);
  m_CurrentOutputs.resize( model->GetNumberOfScalarOutputs() );
  Model::ErrorType error = m->GetScalarOutputsAndLogLikelihood(
    m_CurrentParameters, m_CurrentOutputs, m_CurrentLogLikelihood);
  // initial starting point LogLikelihood.
  assert (error == Model::NO_ERROR);
}


void MetropolisHastingsSampler
::SetStepSize( double stepSize )
{
  m_StepSize = stepSize;
}

Sample
MetropolisHastingsSampler
::NextSample()
{
  // xc is x_candidate
  Model * m = const_cast< Model * >(m_Model);

  std::vector< double > xc( m_Model->GetNumberOfParameters(), 0.0 );
  std::vector< double > yc( m_Model->GetNumberOfScalarOutputs(), 0.0 );

  unsigned int numberOfActiveParameters = this->GetNumberOfActiveParameters();

  assert( std::count( m_ActiveParameterIndices.begin(),
                      m_ActiveParameterIndices.end(), true )
          == numberOfActiveParameters);

  for ( unsigned int i = 0; i < m_Model->GetNumberOfParameters(); i++ ) {
    if ( m_ActiveParameterIndices[i] ) {
      double step
        = (m_StepSize   // scale each step by this variable
           //* m_Random.Gaussian() // random direction, length
           * ( m_Random.Uniform() - 0.5 ) // random direction, length
           * m_StepScales[i]); // scaled by parameter domain size
      xc[i] = m_CurrentParameters[i] + step;
    } else {
      xc[i] = m_CurrentParameters[i];
    }
  }
  double ll; // ll is new_log_likelihood
  m->GetScalarOutputsAndLogLikelihood(xc,yc,ll);

  // Check for NaN
  assert( ll == ll );

  double delta_logLikelihood = ll - m_CurrentLogLikelihood;

  if ((delta_logLikelihood > 0) ||
      (std::exp(delta_logLikelihood) > m_Random.Uniform())) {
    m_CurrentLogLikelihood = ll;
    m_CurrentParameters = xc;
    m_CurrentOutputs = yc;
    return Sample( xc, yc, ll );
  }

  // Stay at this point in parameter space
  return Sample( m_CurrentParameters,
                 m_CurrentOutputs,
                 m_CurrentLogLikelihood );

}

} // end namespace madai
