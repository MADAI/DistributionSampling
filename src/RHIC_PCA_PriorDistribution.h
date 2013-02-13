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

#ifndef __RHIC_PCA_PriorDistribution_h__
#define __RHIC_PCA_PriorDistribution_h__


#include "PriorDistribution.h"


namespace madai {

class RHIC_PCA_PriorDistribution : public PriorDistribution {
public:
  RHIC_PCA_PriorDistribution(Model *in_Model);
  double Evaluate(std::vector<double> Theta);
};


} // end namespace madai

#endif // __RHIC_PCA_PriorDistribution_h__
