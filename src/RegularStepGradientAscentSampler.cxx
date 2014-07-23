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

#include "RegularStepGradientAscentSampler.h"


namespace madai {


RegularStepGradientAscentSampler
::RegularStepGradientAscentSampler() :
  Sampler(),
  m_StepSize( 1.0e-3 ),
  m_Maximize( true )
{
}


RegularStepGradientAscentSampler
::~RegularStepGradientAscentSampler()
{
}


Sample
RegularStepGradientAscentSampler
::NextSample()
{
  std::vector< bool > activeParameters( m_CurrentParameters.size() );

  // TODO - set these from the set of active parameters
  for ( unsigned int i = 0; i < activeParameters.size(); i++ ) {
    activeParameters[i] = true;
    }

  std::vector< double > scalars;
  std::vector< double > gradient;

  Model::ErrorType error =
    m_Model->GetScalarAndGradientOutputs( m_CurrentParameters, activeParameters,
                                          scalars, gradient );
  if (error != Model::NO_ERROR) {
    std::cerr << "In RegularStepGradientAscentSampler::NextSample()\n"
      "  Model::GetScalarAndGradientOutputs() returned error "
              << Model::GetErrorTypeAsString(error) << '\n';
    return Sample();
  }

  // Get the log likelihood
  double logLikelihood = 0.0;
  error =
    m_Model->GetScalarOutputsAndLogLikelihood( m_CurrentParameters, scalars, logLikelihood );

  if (error != Model::NO_ERROR) {
    std::cerr << "In RegularStepGradientAscentSampler::NextSample()\n"
      "  Model::GetScalarOutputsAndLogLikelihood() returned error "
              << Model::GetErrorTypeAsString(error) << '\n';
    return Sample();
  }

  // FIX ME - This is really innefficient calling this again but Model needs to be refactored
  // such that calls can be made sequentially instead of these monolithic functions that
  // share code
  m_Model->GetScalarOutputsAndLogLikelihoodAndLikelihoodErrorGradient(
    m_CurrentParameters, m_CurrentOutputs, logLikelihood, 
    m_CurrentLogLikelihoodValueGradient, m_CurrentLogLikelihoodErrorGradient);
  Sample sample( m_CurrentParameters,
                 m_CurrentOutputs,
                 logLikelihood,
                 m_CurrentLogLikelihoodValueGradient,
                 m_CurrentLogLikelihoodErrorGradient);

  // Update the current parameters to the new position
  double direction = 1.0;
  if ( !m_Maximize ) {
    direction = -1.0;
  }

  unsigned int activeParameter = 0;
  for ( unsigned int i = 0; i < m_CurrentParameters.size(); ++i ) {
    if ( m_ActiveParameters.find( m_Model->GetParameters()[i].m_Name ) !=
         m_ActiveParameters.end() ) {
      m_CurrentParameters[i] = direction * m_StepSize * gradient[activeParameter++] +
        m_CurrentParameters[i];
    }
  }

  return sample;
}


void
RegularStepGradientAscentSampler
::SetStepSize( double stepSize )
{
  m_StepSize = stepSize;
}


double
RegularStepGradientAscentSampler
::GetStepSize() const
{
  return m_StepSize;
}


void
RegularStepGradientAscentSampler
::Minimize()
{
  m_Maximize = false;
}


void
RegularStepGradientAscentSampler
::Maximize()
{
  m_Maximize = true;
}


bool
RegularStepGradientAscentSampler
::GetMaximize() const
{
  return m_Maximize;
}

} // end namespace madai
