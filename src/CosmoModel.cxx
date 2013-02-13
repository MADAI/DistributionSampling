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

#include "CosmoModel.h"

#include "CosmoLikelihoodDistribution.h"
#include "CosmoPriorDistribution.h"


namespace madai {

CosmoModel
::CosmoModel()
{
  this->m_StateFlag = UNINITIALIZED;
}

CosmoModel
::CosmoModel( const std::string info_dir )
{
  this->m_StateFlag = UNINITIALIZED;
  this->LoadConfiguration(info_dir);
}

CosmoModel
::~CosmoModel()
{
}

CosmoModel::ErrorType
CosmoModel
::GetScalarOutputs( const std::vector< double > & parameters,
                    std::vector< double > & scalars ) const
{
  std::stringstream ss;
  std::ifstream inputfile;
  
  ss << "cosmosurvey";
    
  for ( int i = 0; i< m_Parameters.size(); i++ ) {
    ss << " -" << m_Parameters[i].m_Name << " " << parameters[i];
  }
  
  ss << " -nz 10 -nf 10 -ob .0406 > output.dat" << std::endl;
  
  std::cout << ss.str() << std::endl;
  std::cout << "Waiting on cosmosurvey...";
  std::cout.flush();
  int result = system((ss.str()).c_str());
  std::cout << "Done." << std::endl;
  
  inputfile.open("output.dat");
  
  while ( !inputfile.eof() ) {
    double temp;
    inputfile >> temp;
    scalars.push_back( temp );
  }
  return NO_ERROR;
}

// For interaction with the mcmc
CosmoModel::ErrorType
CosmoModel
::GetLikeAndPrior( const std::vector< double > & parameters,
                   double & Like,
                   double & Prior) const
{
  std::vector< double > ModelMeans;
  std::vector< double > ModelErrors;
  this->GetScalarOutputs( parameters, ModelMeans );
  
  Like = m_Likelihood->Evaluate( ModelMeans, ModelErrors );
  Prior = m_Prior->Evaluate( parameters );
  
  return NO_ERROR;
}

CosmoModel::ErrorType
CosmoModel::LoadDistributions()
{
  if( m_UseEmulator || m_ProcessPipe ) {
    std::cerr << "Emulator is not needed for this model. Turn off USE_EMULATOR and PROCESS_PIPE" << std::endl;
    this->m_StateFlag = ERROR;
    return OTHER_ERROR;
  }
  m_Likelihood = new CosmoLikelihoodDistribution( this );
  m_Prior = new CosmoPriorDistribution( this );
}

} // end namespace madai
