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

#ifndef __RHIC_PCA_Model_h__
#define __RHIC_PCA_Model_h__


#include "MultiModel.h"

#include "Quad.h"


namespace madai {

/** \class RHIC_PCA_Model
 *
 * \todo Document the difference between this class and RHICModel
 */  
class RHIC_PCA_Model : public MultiModel {
public:
  RHIC_PCA_Model();
  RHIC_PCA_Model(std::string info_dir);
  virtual ~RHIC_PCA_Model();
    
    
  virtual ErrorType GetScalarOutputs( const std::vector< double > & parameters,
                                      std::vector< double > & scalars ) const;
    
  // Proposed function for interaction with the MCMC
  /** Get the likelihood and prior at the point parameters in parameter space. */
  virtual ErrorType GetLikeAndPrior( const std::vector< double > & parameters,
                                    double & Like,
                                    double & Prior ) const;
  
  virtual ErrorType LoadDistributions();

private:
  QuadHandler* m_Quad;
  
}; // end class RHIC_PCA_Model
  
} // end namespace madai

#endif // end __RHIC_PCA_Model_h__
