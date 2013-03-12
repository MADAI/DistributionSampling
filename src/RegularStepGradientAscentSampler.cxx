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

#include "RegularStepGradientAscentSampler.h"


namespace madai {


RegularStepGradientAscentSampler
::RegularStepGradientAscentSampler() :
  Sampler(),
  m_StepSize( 1.0e-3 ),
  m_Minimize( true )
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

  // Get the log likelihood
  double logLikelihood = 0.0;
  error =
    m_Model->GetScalarOutputsAndLogLikelihood( m_CurrentParameters, scalars, logLikelihood );

  Sample sample( m_CurrentParameters, scalars, logLikelihood );

  // Update the current parameters to the new position
  double direction = 1.0;
  if ( m_Minimize ) {
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


void
RegularStepGradientAscentSampler
::MinimizeOn()
{
  m_Minimize = true;
}


void
RegularStepGradientAscentSampler
::MinimizeOff()
{
  m_Minimize = false;
}

} // end namespace madai
