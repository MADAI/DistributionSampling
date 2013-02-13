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

#ifndef __CosmoModel_h__
#define __CosmoModel_h__

#include "MultiModel.h"

namespace madai {

/** \class CosmoModel
 *
 * Encapsulates the Cosmo simulation. */
class CosmoModel : public MultiModel {
public:
  CosmoModel();
  CosmoModel(std::string info_dir);
  virtual ~CosmoModel();


  virtual ErrorType GetScalarOutputs( const std::vector< double > & parameters,
                                      std::vector< double > & scalars ) const;

  // Proposed function for interaction with the MCMC
  /** Get the likelihood and prior at the point parameters in parameter space. */
  virtual ErrorType GetLikeAndPrior( const std::vector< double > & parameters,
                                    double & Like,
                                    double & Prior ) const;

  virtual ErrorType LoadDistributions();

}; // end class CosmoModel

} // end namespace madai

#endif // end __CosmoModel_h__
