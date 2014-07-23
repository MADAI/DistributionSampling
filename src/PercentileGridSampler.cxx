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

#include "PercentileGridSampler.h"

#include <cassert> // assert
#include <cmath> // std::exp
#include <algorithm> // std::count

namespace madai {


PercentileGridSampler
::PercentileGridSampler() :
  Sampler(),
  m_NumberOfSamplesInEachDimension(4)
{
}


PercentileGridSampler
::~PercentileGridSampler()
{
}

Sample
PercentileGridSampler
::NextSample()
{
  const std::vector< Parameter > & parameters = m_Model->GetParameters();
  assert (this->GetNumberOfActiveParameters() > 0);
  unsigned int p = m_Model->GetNumberOfParameters();

  assert(m_CurrentParameters.size() == m_Model->GetNumberOfParameters());

  // do something
  double rangeOverN = 1.0 / static_cast< double >( m_NumberOfSamplesInEachDimension );
  double start = 0.5 * rangeOverN;
  for ( unsigned int dim = 0; dim < p; ++dim ) {
    if (this->IsParameterActive(dim)) {
      m_CurrentParameters[dim] =
        parameters[dim].GetPriorDistribution()->GetPercentile(
            start + (m_StateVector[dim] * rangeOverN));
    }
  }

  unsigned int dim = 0;
  while ((! this->IsParameterActive(dim)) ||
         (m_StateVector[dim] == m_NumberOfSamplesInEachDimension - 1)) {
    m_StateVector[dim] = 0;
    dim = (dim + 1) % p;
  }
  m_StateVector[dim] ++;

  std::vector< double > y( m_Model->GetNumberOfScalarOutputs(), 0.0 );
  Model * m = const_cast< Model * >(m_Model);
  m->GetScalarOutputsAndLogLikelihoodAndLikelihoodErrorGradient(
    m_CurrentParameters, m_CurrentOutputs, m_CurrentLogLikelihood, 
    m_CurrentLogLikelihoodValueGradient, m_CurrentLogLikelihoodErrorGradient);
  return Sample( m_CurrentParameters,
                 m_CurrentOutputs,
                 m_CurrentLogLikelihood,
                 m_CurrentLogLikelihoodValueGradient,
                 m_CurrentLogLikelihoodErrorGradient);
}

void PercentileGridSampler
::SetNumberOfSamples( unsigned int N )
{
  if (m_Model == NULL)
    return;
  // call this function *after* Deactivating Parameters!
  unsigned int p = this->GetNumberOfActiveParameters();
  assert((p <= m_Model->GetNumberOfParameters()) && (p > 0));
  unsigned int n = static_cast< unsigned int >(
      std::ceil(std::pow(N,1.0 / static_cast< double >(p))));
  if (n < 2)
    n = 2;
  m_NumberOfSamplesInEachDimension = n;
}

unsigned int
PercentileGridSampler
::GetNumberOfSamples()
{
  if ( this->GetNumberOfActiveParameters() == 0 ) {
    return 0;
  }

  float floatResult = std::pow(static_cast<float>(m_NumberOfSamplesInEachDimension),
                               static_cast<int>(this->GetNumberOfActiveParameters()));
  return static_cast<unsigned int>( floatResult + 0.5 );
}

void
PercentileGridSampler
::Reset()
{
  this->Initialize( m_Model );
}

void
PercentileGridSampler
::Initialize( const Model * model )
{
  assert(model != NULL);
  if ((model == NULL) || (m_Model == model))
    return;
  m_Model = model;

  Sampler::Initialize( model );
  unsigned int p = m_Model->GetNumberOfParameters();
  this->SetNumberOfSamples(this->GetNumberOfSamples());
  m_StateVector.clear();
  m_StateVector.resize(p,0);
}

} // end namespace madai
