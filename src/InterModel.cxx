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

// Not sure if this is working as I don't have the CRHICStat stuff installed yet

#include "InterModel.h"

namespace madai {

InterModel
::InterModel()
{
  this->m_Emulator = NULL;
  this->stateFlag = UNINITIALIZED;
}

InterModel
::InterModel( const std::string info_dir )
{
  this->m_Emulator = NULL;
  this->stateFlag = UNINITIALIZED;
  this->LoadConfiguration(info_dir);
}

InterModel
::~InterModel()
{
}

InterModel::ErrorType
InterModel::GetScalarOutputs( const std::vector< double > & parameters,
                              std::vector< double > & scalars ) const
{
  clock_t begintime;
  double likelihood;
  double *x = new double[parameters.size()]();
  for ( unsigned int j = 0; j < parameters.size(); j++ ) {
    x[j] = parameters[j];
  }

  if ( m_Verbose ) {
    std::cerr << "Theta: ";
    for ( int i = 0; i < parameters.size(); i++ ) {
      std::cerr << parameters[i] << " ";
    }
    std::cerr << std::endl;
  }

  if ( m_Timing ) {
    begintime = clock();
  }

  if ( m_UseEmulator ) {
    //likelihood = m_Emulator->GetLL(x)
    //std::cerr << "Using 'x': " << likelihood << std::endl;
    likelihood = m_Emulator->GetLL(&parameters[0]);
    //std::cerr << "Using '&parameters[0]': " << likelihood << std::endl;
  }
  scalars.push_back( likelihood );

  if ( m_Timing ) {
    std::cerr << "Likelihood evaluation took " << (clock() - begintime)*1000/CLOCKS_PER_SEC << " ms." << std::endl;
  }
  return NO_ERROR;
}

// For interaction with the mcmc
InterModel::ErrorType
InterModel::GetLikeAndPrior( const std::vector< double > & parameters,
                             double & Like,
                                    double & Prior) const
{
  std::vector< double > tempv;
  this->GetScalarOutputs( parameters, tempv );
  if ( tempv.size() != 1 ) {
    std::cerr << "Size mismatch for getting likelihood from model" << std::endl;
    return OTHER_ERROR;
  } else {
    Like = tempv[0];
  }

  Prior = m_Prior->Evaluate(parameters);

  return NO_ERROR;
}

InterModel::ErrorType
InterModel::LoadDistributions()
{
  parameterMap * parmap;
  bool smap = parameter::getB( m_ParameterMap, "LIKELIHOOD_PARAMETER_MAP", false );
  if ( smap ) {
    std::string parmapfile = m_DirectoryName + "/defaultpars/likelihood.param";
    parmap = new parameterMap;
    parameter::ReadParsFromFile( *parmap, parmapfile );
  } else {
    parmap = &m_ParameterMap;
  }

  if ( m_UseEmulator ) {
    std::cerr << "Emulator is being loaded from: " << m_DirectoryName << std::endl;
    m_Emulator = new CRHICStat( m_DirectoryName );
  } else {
    std::cerr << "The UseEmulator flag is set to false ( or not set). We can't do anything without an emulator" << std::endl;
    this->stateFlag = ERROR;
    return OTHER_ERROR;
  }

  m_Prior = new PriorDistribution_Interpolator( this );

  return NO_ERROR;
}

} // end namespace madai
