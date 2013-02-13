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

#include "LikelihoodDistribution.h"

namespace madai {


LikelihoodDistribution
::LikelihoodDistribution() :
  m_UseEmulator( false ),
  m_ProcessPipe( false )
{
}


LikelihoodDistribution
::~LikelihoodDistribution()
{
}


double
LikelihoodDistribution
::Evaluate( std::vector<double> ModelMeans,
            std::vector<double> ModelErrors )
{
}


std::vector< double >
LikelihoodDistribution
::GetData()
{
}

} // end namespace madai
