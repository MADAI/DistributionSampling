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

#include "GaussianDistribution.h"

#include <cmath>

namespace madai {

GaussianDistribution
::GaussianDistribution() :
  m_Mean( 0.0 ),
  m_StandardDeviation( 1.0 ),
  m_InternalDistribution( new boost::math::normal( m_Mean, m_StandardDeviation ) )
{
}


GaussianDistribution
::~GaussianDistribution()
{
  delete m_InternalDistribution;
}

Distribution *
GaussianDistribution
::Clone() const
{
  GaussianDistribution * d = new GaussianDistribution();
  d->SetMean(this->GetMean());
  d->SetStandardDeviation(this->GetStandardDeviation());
  return d;
}


void
GaussianDistribution
::SetMean( double mean )
{
  m_Mean = mean;

  delete m_InternalDistribution;
  m_InternalDistribution = new boost::math::normal( m_Mean, m_StandardDeviation );

}


double
GaussianDistribution
::GetMean() const
{
  return m_Mean;
}


void
GaussianDistribution
::SetStandardDeviation( double standardDeviation )
{
  m_StandardDeviation = standardDeviation;

  delete m_InternalDistribution;
  m_InternalDistribution = new boost::math::normal( m_Mean, m_StandardDeviation );

}


double
GaussianDistribution
::GetStandardDeviation() const
{
  return m_StandardDeviation;
}


double
GaussianDistribution
::GetLogProbabilityDensity( double value ) const
{
  return log( this->GetNormalizationFactor() ) + this->GetExponent( value );
}


double
GaussianDistribution
::GetProbabilityDensity( double value ) const
{
  //return this->GetNormalizationFactor() * exp( this->GetExponent( value ) );
  return boost::math::pdf( *m_InternalDistribution, value );
}


double
GaussianDistribution
::GetPercentile( double percentile ) const
{
  return boost::math::quantile( *m_InternalDistribution, percentile );
}

double
GaussianDistribution
::GetSample(madai::Random & r) const
{
  return r.Gaussian(this->m_Mean, this->m_StandardDeviation);
}

double
GaussianDistribution
::GetNormalizationFactor() const
{
  double variance = m_StandardDeviation * m_StandardDeviation;

  return (1.0 / sqrt( 2.0 * M_PI * variance ) );
}


double
GaussianDistribution
::GetExponent( double value ) const
{
  double variance = m_StandardDeviation * m_StandardDeviation;
  double exponent = -( value - m_Mean ) * ( value - m_Mean ) / ( 2.0 * variance );

  return exponent;
}


} // end namespace madai
