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

#include "UniformDistribution.h"
#include "Random.h"

#include <cmath>
#include <limits>


namespace madai {

UniformDistribution
::UniformDistribution() :
  m_Minimum( 0.0 ),
  m_Maximum( 1.0 )
{
}


UniformDistribution
::~UniformDistribution()
{
}


void
UniformDistribution
::SetMinimum( double minimum )
{
  m_Minimum = minimum;
}


double
UniformDistribution
::GetMinimum() const
{
  return m_Minimum;
}

void
UniformDistribution
::SetMaximum( double maximum )
{
  m_Maximum = maximum;
}


double
UniformDistribution
::GetMaximum() const
{
  return m_Maximum;
}

double
UniformDistribution
::GetLogProbabilityDensity( double value ) const
{
  if ( this->InRange( value ) ) {
    return log( this->GetProbabilityDensity( value ) );
  } else {
    return -std::numeric_limits< double >::infinity();
  }
}


double
UniformDistribution
::GetProbabilityDensity( double value ) const
{
  if ( this->InRange( value ) ) {
    return 1.0 / ( m_Maximum - m_Minimum );
  } else {
    return 0.0;
  }
}


double
UniformDistribution
::GetPercentile( double percentile ) const
{
  return percentile * ( m_Maximum - m_Minimum ) + m_Minimum;
}

double
UniformDistribution
::GetSample(madai::Random & r) const
{
  return r.Uniform(m_Minimum, m_Maximum);
}


bool
UniformDistribution
::InRange( double value ) const {
  return ( value >= m_Minimum && value <= m_Maximum );
}

} // end namespace madai
