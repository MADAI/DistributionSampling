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

#ifndef __TestLikelihoodDistribution_h__
#define __TestLikelihoodDistribution_h__

#include "LikelihoodDistribution.h"


namespace madai {


class TestLikelihoodDistribution : public LikelihoodDistribution {
public:
  TestLikelihoodDistribution(Model *in_Model);
  ~TestLikelihoodDistribution();
  double Evaluate(std::vector<double> ModelMeans,
                  std::vector<double> ModelErrors);
  
private:
  std::vector<double> GetData();
  
  std::vector<double> m_Data;
};

} // end namespace madai

#endif // __TestLikelihoodDistribution_h__
