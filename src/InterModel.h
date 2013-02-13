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

#ifndef __InterModel_h__
#define __InterModel_h__

#include "MultiModel.h"
#include "rhicstat.h"

namespace madai {

/** \class InterModel
 *
 */
class InterModel : public MultiModel {
private:
  bool       m_Verbose;
  bool       m_Timing;
  CRHICStat* m_Emulator;

public:
  InterModel();
  InterModel(std::string info_dir);
  virtual ~InterModel();

  virtual ErrorType GetScalarOutputs( const std::vector< double > & parameters,
                                      std::vector< double > & scalars ) const;

  // For interaction with the MCMC
  virtual ErrorType GetLikeAndPrior(const std::vector< double > & parameters,
                                    double & Like,
                                    double & Prior ) const;

  virtual ErrorType LoadDistributions();

}; // end class InterModel

} // end namespace madai

#endif // __InterModel_h__
