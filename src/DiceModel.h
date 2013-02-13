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

#ifndef __DiceModel_h__
#define __DiceModel_h__

#include <vector>
#include <string>
#include <map>
#include <math.h>
#include <iostream>
#include <fstream>

#include "parametermap.h"
#include "Model.h"

namespace madai {
  
/** \class DiceModel
 *
 * Example model of a fair die.
 */
class DiceModel : public Model {
public:
  bool         m_Sum;
  bool         m_Distinguishable;
  bool         m_Weighted;
  int          m_Denom;
  int          m_Sides;
  std::string  m_ConfigFile;
  
  bool good() { return (this->m_StateFlag == READY);}
  
  DiceModel();
  DiceModel(const std::string info_dir);
  virtual ~DiceModel() {};
  
  virtual ErrorType LoadConfigurationFile( const std::string info_dir );
  
  virtual ErrorType GetScalarOutputs( const std::vector< double > & parameters,
                                      std::vector< double > & scalars ) const;
                                      
  virtual ErrorType GetLikeAndPrior( const std::vector< double > & parameters,
                                     double & Like,
                                     double & Prior) const;

protected:
  unsigned int m_NumberOfParameters, m_NumberOfOutputs;
  std::vector< std::vector< double > > m_Weights;
  std::vector< double >                m_TotalWeights;

  typedef enum {
    UNINITIALIZED,
    READY,
    ERROR
  } InternalState;
  InternalState m_StateFlag;
  
  double GetWeightedDiceLikelihood( const std::vector< double > & outputs ) const;
  double GetRegularDiceLikelihood( const std::vector< double > & outputs ) const;
  
}; // end class DiceModel

} // end namespace madai

#endif // end __DiceModel_h__
