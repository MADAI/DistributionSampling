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

#include "TestModel.h"

#include "TestLikelihoodDistribution.h"
#include "TestPriorDistribution.h"


namespace madai {


TestModel
::TestModel()
{
  this->m_StateFlag = UNINITIALIZED;
}


TestModel
::TestModel( const std::string info_dir )
{
  this->m_StateFlag = UNINITIALIZED;
  this->LoadConfiguration( info_dir );
}


TestModel
::~TestModel()
{
}


TestModel::ErrorType
TestModel
::GetScalarOutputs( const std::vector< double > & parameters,
                    std::vector< double > & scalars ) const
{
  std::vector< double > Means;
  std::vector< double > Errors;

  if ( m_UseEmulator ) {
    std::cerr << "TestModel should not use an emulator. Set the UseEmulator flag to false" << std::endl;
    return OTHER_ERROR;
  } else {
    double errors[] = {0,0,0,0};
    Errors.assign( errors,errors + 4 );
    double means[4];
    for ( int i = 0; i < 4; i++ ) {
      means[i] = parameters[i];
    }
    Means.assign( means,means + 4 );
  }
  for ( int j = 0; j < 4; j++ ) {
    scalars.push_back( Means[j] );
    scalars.push_back( Errors[j] );
  }

  return NO_ERROR;
}


TestModel::ErrorType
TestModel
::GetLikeAndPrior( const std::vector< double > & parameters,
                   double & Like, double & Prior) const
{
  std::vector< double > outputs;
  std::vector< double > ModelMeans;
  std::vector< double > ModelErrors;

  this->GetScalarOutputs( parameters, outputs );
  for ( unsigned int i = 0; i < outputs.size(); i++ ) {
    if ( i % 2 == 0 ) {
      ModelMeans.push_back( outputs[i] );
    } else {
      ModelErrors.push_back( outputs[i] );
    }
  }

  Like = m_Likelihood->Evaluate( ModelMeans, ModelErrors );
  Prior = m_Prior->Evaluate( parameters );

  return NO_ERROR;
}


TestModel::ErrorType
TestModel
::LoadDistributions()
{
  if ( m_UseEmulator || m_ProcessPipe ) {
    std::cerr << "Emulator is not used for the test model. Turn off USE_EMULATOR and PROCESS_PIPE" << std::endl;
    this->m_StateFlag = ERROR;
    return OTHER_ERROR;
  }
  m_Likelihood = new TestLikelihoodDistribution( this );
  m_Prior = new TestPriorDistribution( this );
}

} // end namespace madai
