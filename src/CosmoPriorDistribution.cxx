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

#include "CosmoPriorDistribution.h"

#include "Model.h"


namespace madai {

CosmoPriorDistribution
::CosmoPriorDistribution( Model * in_Model )
{
  m_Model = in_Model;
  m_SepMap = parameter::getB( m_Model->m_ParameterMap, "PRIOR_PARAMETER_MAP", false );

  if ( m_SepMap ) {
    std::string parmapfile = m_Model->m_DirectoryName + "/parameters/prior.param";
    m_ParameterMap = new parameterMap;
    parameter::ReadParsFromFile( *m_ParameterMap, parmapfile );
    //parameter::ReadParsFromFile(parmap, parameter_file_name);
  } else {
    m_ParameterMap = &(m_Model->m_ParameterMap);
  }
}


double
CosmoPriorDistribution
::Evaluate( std::vector< double > Theta )
{
  return 1.0;
}

} // end namespace madai
