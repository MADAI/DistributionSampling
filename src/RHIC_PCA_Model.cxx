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

#include "RHIC_PCA_Model.h"

#include "RHIC_PCA_LikelihoodDistribution.h"
#include "RHIC_PCA_PriorDistribution.h"

namespace madai {

RHIC_PCA_Model
::RHIC_PCA_Model()
{
  this->m_StateFlag = UNINITIALIZED;
  this->m_Quad = NULL;
}


RHIC_PCA_Model
::RHIC_PCA_Model( const std::string info_dir )
{
  this->m_StateFlag = UNINITIALIZED;
  this->m_Quad = NULL;
  this->LoadConfiguration( info_dir );
}


RHIC_PCA_Model
::~RHIC_PCA_Model()
{
  if ( m_Quad != NULL ) {
    delete m_Quad;
  }
}


RHIC_PCA_Model::ErrorType
RHIC_PCA_Model
::GetScalarOutputs( const std::vector< double > & parameters,
                    std::vector< double > & scalars ) const
{
  std::vector< double > Means;
  std::vector< double > Errors;

  if ( m_UseEmulator ) {
    m_Quad->QueryQuad( parameters, Means, Errors );
  } else {
    // determine another way to fill the vectors
  }
  if ( Means.size() != Errors.size() ) {
    std::cerr << "Means and Errors from PCA Model arn't the same size" << std::endl;
    return OTHER_ERROR;
  } else {
    for ( unsigned int i = 0; i < Means.size(); i++ ) {
      scalars.push_back( Means[i] );
      scalars.push_back( Errors[i] );
    }
  }
  return NO_ERROR;
}


// For interaction with the mcmc
RHIC_PCA_Model::ErrorType
RHIC_PCA_Model
::GetLikeAndPrior( const std::vector< double > & parameters,
                   double & Like, double & Prior) const
{
  std::vector< double > outputs;
  std::vector< double > ModelMeans;
  std::vector< double > ModelErrors;

  this->GetScalarOutputs( parameters, outputs );
  for ( unsigned int i = 0; i < outputs.size(); i++ ) {
    if( i % 2 == 0 ) {
      ModelMeans.push_back( outputs[i] );
    } else {
      ModelErrors.push_back( outputs[i] );
    }
  }

  Like = m_Likelihood->Evaluate( ModelMeans, ModelErrors );
  Prior = m_Prior->Evaluate( parameters );

  return NO_ERROR;
}


RHIC_PCA_Model::ErrorType
RHIC_PCA_Model
::LoadDistributions()
{
  if ( m_UseEmulator ) {
    bool sep_map = parameter::getB( m_ParameterMap, "LIKELIHOOD_PARAMETER_MAP", false );
    if ( sep_map ) {
      std::string pmf = m_DirectoryName + "/defaultpars/likelihood.param";
      parameterMap* ParMap = new parameterMap;
      parameter::ReadParsFromFile( *ParMap, pmf );
      m_Quad = new QuadHandler( ParMap, this );
    } else {
      m_Quad = new QuadHandler( &m_ParameterMap, this );
    }
  } else {
    std::cerr << "RHIC_PCA requires the use of the Quad emulator" << std::endl;
    this->m_StateFlag = ERROR;
    return OTHER_ERROR;
  }

  m_Likelihood = new RHIC_PCA_LikelihoodDistribution( this );
  m_Prior = new RHIC_PCA_PriorDistribution( this );
}

} // end namespace madai
