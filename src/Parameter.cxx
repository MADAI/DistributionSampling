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

#include "Parameter.h"
#include "UniformDistribution.h"

namespace madai {

Parameter::Parameter( ) :
  m_PriorDistribution( NULL )
{
}

Parameter::Parameter( std::string name) :
  m_Name( name )
{
  UniformDistribution * u = new UniformDistribution();
  u->SetMinimum( 0.0 );
  u->SetMaximum( 1.0 );
  m_PriorDistribution = u;
}


Parameter::Parameter( std::string name, double minimum, double maximum ) :
  m_Name( name )
{
  UniformDistribution * u = new UniformDistribution();
  u->SetMinimum( minimum );
  u->SetMaximum( maximum );
  m_PriorDistribution = u;
}


Parameter::Parameter( std::string name, const Distribution & distribution) :
  m_Name( name ),
  m_PriorDistribution( distribution.Clone() )
{
}


Parameter::~Parameter()
{
  delete m_PriorDistribution;
}


Parameter::Parameter( const Parameter & other) :
  m_Name( other.m_Name ),
  m_PriorDistribution( other.m_PriorDistribution->Clone() )
{
}


Parameter & Parameter::operator=( const Parameter & other)
{
  m_Name = other.m_Name;
  if ( m_PriorDistribution != NULL )
    delete m_PriorDistribution;
  m_PriorDistribution = other.m_PriorDistribution->Clone();
}


const Distribution * Parameter::GetPriorDistribution() const
{
  return m_PriorDistribution;
}

} // namespace madai
