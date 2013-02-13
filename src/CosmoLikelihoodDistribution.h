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

#ifndef __CosmoLikelihoodDistribution_h__
#define __CosmoLikelihoodDistribution_h__

#include "LikelihoodDistribution.h"

namespace madai {

class CosmoLikelihoodDistribution : public LikelihoodDistribution {
public:
  CosmoLikelihoodDistribution(Model *in_Model);
  ~CosmoLikelihoodDistribution();
  double Evaluate(std::vector<double> ModelMeans,
                  std::vector<double> ModelErrors);

private:
  std::vector< double > GetData();
  
  std::vector< double > m_Data;
  std::vector< int >    m_IntData;
};


} // end namespace madai

#endif // __CosmoLikelihoodDistribution_h__
