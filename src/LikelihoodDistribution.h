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

#ifndef __LikelihoodDistribution_h__
#define __LikelihoodDistribution_h__

#include "Distribution.h"


namespace madai {

class LikelihoodDistribution : public Distribution {
public:
  LikelihoodDistribution();
  virtual ~LikelihoodDistribution();
  virtual double Evaluate(std::vector<double> ModelMeans,
                          std::vector<double> ModelErrors);
  
protected:
  virtual std::vector<double> GetData();

  std::vector<double> m_Data;
  bool                m_UseEmulator;
  bool                m_ProcessPipe;
  std::ofstream       m_EmulatorTest;
};


} // end namespace madai


#endif // __LikelihoodDistribution_h__
