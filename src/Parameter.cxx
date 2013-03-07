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

#include "Parameter.h"
#include "UniformDistribution.h"

namespace madai {

Parameter::Parameter( std::string nm) :
  m_Name(nm),
  m_MinimumPossibleValue(0.0),
  m_MaximumPossibleValue(1.0)
{
  UniformDistribution * u = new UniformDistribution();
  u->SetMinimum( 0.0 );
  u->SetMaximum( 1.0 );
  this->m_PriorDistribution = u;
}

Parameter::Parameter( std::string nm, double mn, double mx) :
  m_Name(nm),
  m_MinimumPossibleValue(mn),
  m_MaximumPossibleValue(mx)
{
  UniformDistribution * u = new UniformDistribution();
  u->SetMinimum( mn );
  u->SetMaximum( mx );
  this->m_PriorDistribution = u;
}

Parameter::Parameter( std::string nm, const Distribution & distribution) :
  m_Name(nm),
  m_PriorDistribution(distribution.Clone())
{
  this->m_MinimumPossibleValue = 0.0; //FIXME
  this->m_MaximumPossibleValue = 1.0; //FIXME
}

Parameter::~Parameter()
{
  delete this->m_PriorDistribution;
}

Parameter::Parameter( const Parameter & other) :
  m_Name(other.m_Name),
  m_MinimumPossibleValue(other.m_MinimumPossibleValue),
  m_MaximumPossibleValue(other.m_MaximumPossibleValue),
  m_PriorDistribution(other.m_PriorDistribution->Clone())
{
}

Parameter & Parameter::operator=( const Parameter & other)
{
  this->m_Name = other.m_Name;
  this->m_MinimumPossibleValue = other.m_MinimumPossibleValue;
  this->m_MaximumPossibleValue = other.m_MaximumPossibleValue;
  this->m_PriorDistribution = other.m_PriorDistribution->Clone();
}

const Distribution * Parameter::GetPriorDistribution() const
{
  return this->m_PriorDistribution;
}

} // namespace madai
