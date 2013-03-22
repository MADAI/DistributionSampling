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

#include "UniformSampler.h"

#include <cassert> // assert
#include <cmath> // std::exp
#include <algorithm> // std::count

namespace madai {


UniformSampler
::UniformSampler() :
  Sampler(),
  m_SampleScale(4)
{
}


UniformSampler
::~UniformSampler()
{
}

void
UniformSampler
::Initialize( const Model * model )
{
  assert(model != NULL);
  m_Model = model;

  Sampler::Initialize( model );
  unsigned int p = m_Model->GetNumberOfParameters();
  this->SetNumberSamples(this->GetNumberSamples());
  m_StateVector.clear();
  m_StateVector.resize(p,0);
}


unsigned int
UniformSampler
::GetNumberSamples()
{
  return std::pow(m_SampleScale,m_Model->GetNumberOfParameters());
}


void UniformSampler
::SetNumberSamples( unsigned int N )
{
  if (m_Model == NULL)
    return;
  unsigned int p = m_Model->GetNumberOfParameters();
  unsigned int n = static_cast< unsigned int >(
      std::ceil(std::pow(N,1.0 / static_cast< double >(p))));
  if (n < 2)
    n = 2;
  m_SampleScale = n;
}

Sample
UniformSampler
::NextSample()
{
  Model * m = const_cast< Model * >(m_Model);
  unsigned int p = m_Model->GetNumberOfParameters();

  // do something
  std::vector< double > x(p,0.0);

  double rangeOverN = 1.0 / static_cast< double >( m_SampleScale );
  double start = 0.5 * rangeOverN;
  for ( int dim = 0; dim < p; ++dim ) {
    x[dim] =
      m_Model->GetParameters()[dim].GetPriorDistribution()
      ->GetPercentile(start + (m_StateVector[dim] * rangeOverN));
  }

  unsigned int dim = 0;
  while (m_StateVector[dim] == m_SampleScale - 1) {
    m_StateVector[dim] = 0;
    dim = (dim + 1) % p;
  }
  m_StateVector[dim] ++;

  std::vector< double > y( m_Model->GetNumberOfScalarOutputs(), 0.0 );
  double ll; // ll is new_log_likelihood
  m->GetScalarOutputsAndLogLikelihood(x,y,ll);
  return Sample( x, y, ll );

}

} // end namespace madai
