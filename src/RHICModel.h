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

#ifndef __RHICModel_h__
#define __RHICModel_h__

#include "MultiModel.h"

#include "EmuPlusPlus/EmuPlusPlus.h"
#include "process_pipe.h"


namespace madai {


/** \class RHICModel
 *
 * Adapter to the RHIC model. */
class RHICModel : public MultiModel {
public:
  RHICModel();
  RHICModel(std::string info_dir);
  virtual ~RHICModel();


  virtual ErrorType GetScalarOutputs( const std::vector< double > & parameters,
                                     std::vector< double > & scalars ) const;

  // For interaction with the MCMC
  /** Get the likelihood and prior at the point parameters in parameter space. */
  virtual ErrorType GetLikeAndPrior( const std::vector< double > & parameters,
                                    double & Like,
                                    double & Prior ) const;

  virtual ErrorType LoadDistributions();
  virtual ErrorType LoadProcess();

private:
  process_pipe m_Process;
  emulator*    m_Emulator;

}; // end class RHICModel

} // end namespace madai

#endif // end __RHICModel_h__
