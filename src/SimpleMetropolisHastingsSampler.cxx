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

#include "SimpleMetropolisHastingsSampler.h"

#include <cassert> // assert
#include <cmath> // std::exp
#include <algorithm> // std::count

namespace madai {


SimpleMetropolisHastingsSampler
::SimpleMetropolisHastingsSampler( const Model * model ) :
  Sampler( model ),
  m_StepSize( 1.0e-2 ),
  m_NumberOfParameters( model->GetNumberOfParameters() ),
  m_NumberOfOutputs( model->GetNumberOfScalarOutputs() )
{
  assert(model != NULL);
  const std::vector< Parameter > & params = this->m_Model->GetParameters();
  this->m_stepScales.resize(this->m_NumberOfParameters);
  for ( unsigned int i = 0; i < m_NumberOfParameters; i++ ) {
    const Distribution * priorDist = params[i].GetPriorDistribution();
    this->m_CurrentParameters[i] = priorDist->GetSample(this->m_random);
    // Random initial starting point
    this->m_stepScales[i]
      = priorDist->GetPercentile(0.75) - priorDist->GetPercentile(0.25);
    // set step scales
  }
  Model * m = const_cast< Model * >(m_Model);
  std::vector< double > y(this->m_NumberOfOutputs);
  m->GetScalarOutputsAndLogLikelihood(
    this->m_CurrentParameters, y, this->m_CurrentLogLikelihood);
  // initial starting point LogLikelihood.
}


SimpleMetropolisHastingsSampler
::~SimpleMetropolisHastingsSampler()
{
}


void SimpleMetropolisHastingsSampler
::SetStepSize( double stepSize )
{
  m_StepSize = stepSize;
}

Sample
SimpleMetropolisHastingsSampler
::NextSample()
{
  // xc is x_candidate
  Model * m = const_cast< Model * >(m_Model);

  std::vector< double > xc( m_NumberOfParameters, 0.0 );
  std::vector< double > yc( m_NumberOfOutputs, 0.0 );

  unsigned int numberOfActiveParameters = this->GetNumberOfActiveParameters();

  assert( std::count( this->m_ActiveParameterIndices.begin(),
                      this->m_ActiveParameterIndices.end(), true )
          == numberOfActiveParameters);

  for ( unsigned int giveup = 1048576; giveup != 0; --giveup ) {
    unsigned int k = 0;
    for ( unsigned int i = 0; i < m_NumberOfParameters; i++ ) {
      if ( m_ActiveParameterIndices[i] ) {
        double step
          = (this->m_StepSize   // scale each step by this variable
             * this->m_random.Gaussian() // random direction, length
             * this->m_stepScales[i]); // scaled by parameter domain size
        xc[i] = this->m_CurrentParameters[i] + step;
      } else {
        xc[i] = this->m_CurrentParameters[i];
      }
    }
    double ll; // ll is new_log_likelihood
    m->GetScalarOutputsAndLogLikelihood(xc,yc,ll);
    double delta_logLikelihood = ll - this->m_CurrentLogLikelihood;

    if ((delta_logLikelihood > 0) ||
        (std::exp(delta_logLikelihood) > this->m_random.Uniform())) {
      this->m_CurrentLogLikelihood = ll;
      this->m_CurrentParameters = xc;
      return Sample( xc, yc, ll );
    } // else, loop.
  }
}

} // end namespace madai
