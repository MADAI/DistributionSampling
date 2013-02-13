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

#include "Model.h"

#include "MultiModel.h"

namespace madai {

ProposalDistribution
::ProposalDistribution( Model * in_Model )
{
  m_Model = in_Model;
  std::string type, param_name;
  int count = 0;
  int numparams = m_Model->GetNumberOfParameters();
  std::vector< double > temp( numparams, .01 );

  m_SepMap = parameter::getB( m_Model->m_ParameterMap, "PROPOSAL_PARAMETER_MAP", false );

  if ( m_SepMap ) {
    std::string parmapfile = m_Model->m_DirectoryName + "/defaultpars/proposal.param";
    m_ParameterMap = new parameterMap;
    parameter::ReadParsFromFile( *m_ParameterMap, parmapfile );
  } else {
    m_ParameterMap = &(m_Model->m_ParameterMap);
  }

  m_RescaledMethod    = parameter::getB( *m_ParameterMap, "RESCALED_PROPOSAL", true );
  m_MixingStdDev      = parameter::getV( *m_ParameterMap, "MIXING_STD_DEV", "0" );
  m_SymmetricProposal = parameter::getB( *m_ParameterMap, "SYMMETRIC_PROPOSAL", true );
  m_Prefactor         = parameter::getD( *m_ParameterMap, "PREFACTOR", 1.0 );
  m_MinScale          = parameter::getD( *m_ParameterMap, "MIN", 0.0 );
  m_MaxScale          = parameter::getD( *m_ParameterMap, "Max", 1.0 );
  m_Scale             = parameter::getD( *m_ParameterMap, "SCALE", 1.0 );
  //m_Offset            = parameter::getD( *m_ParameterMap, "OFFSET", 0.0 );

  const gsl_rng_type * rngtype;
  rngtype = gsl_rng_default;
  gsl_rng_env_setup();
  m_RandNumGen = gsl_rng_alloc( rngtype );
  gsl_rng_set( m_RandNumGen, time( NULL ) );
}


int
ProposalDistribution
::FindParam( std::string name )
{
  std::vector< Parameter > Pars = m_Model->GetParameters();
  int out = -1;
  int i = 0;
  bool Found = false;

  while ( i < Pars.size() ) {
    // std::cout << "FindParam: Comparing " << name << " to " << PNames[i] << std::endl;
    if ( strcmp(Pars[i].m_Name.c_str(), name.c_str() ) == 0 ) {
      if ( !Found ) {
        out = i;
        Found = true;
      } else { //A matching parameter has already been found, multiple parameters with the same name.
        std::cout << Pars[out].m_Name << std::endl;
        std::cout << Pars[i].m_Name << std::endl;
        std::cout << "In ProposalName::FindParam; Duplicate parameter names found. Please change parameter names." << std::endl;
        exit( 1 );
      }
    }
    i++;
  }

  return out;
}


void
ProposalDistribution
::SetActiveParameters( std::set< std::string > activeParameters ){
  m_ActiveParameters = activeParameters;
}


std::vector< double >
ProposalDistribution
::Iterate( std::vector<double>& current, double& scale
           /* std::set<std::string>& activeParameters*/ ) {
  std::vector< double > proposed = current;
  if ( m_SymmetricProposal ) {
    //We use the scale set in the parameter file
    double range[2];
    for ( int i = 0; i < proposed.size(); i++ ) {
      if ( m_ActiveParameters.find( m_Model->GetParameters()[i].m_Name ) != m_ActiveParameters.end() ) {
        m_Model->GetRange( i, range );
        proposed[i] = (current[i] - range[0])/(range[1]-range[0]); //scale to between 0 and 1
        proposed[i] = proposed[i] + m_Prefactor*gsl_ran_gaussian(m_RandNumGen, m_Scale*m_MixingStdDev[i]);
        proposed[i] = proposed[i] - floor( proposed[i] );
        proposed[i] = (proposed[i]*(range[1]-range[0]))+range[0];
      }
    }
  } else {
    // We use whatever scale we just got passed from the rest of the code
    scale = (scale*(m_MaxScale-m_MinScale))+m_MinScale;
    double range[2];
    for ( int i = 0; i < proposed.size(); i++ ) {
      if ( m_ActiveParameters.find( m_Model->GetParameters()[i].m_Name ) != m_ActiveParameters.end() ) {
        m_Model->GetRange( i, range );
        proposed[i] = (current[i] - range[0])/(range[1]-range[0]); //scale to between 0 and 1
        proposed[i] = proposed[i] + m_Prefactor*gsl_ran_gaussian(m_RandNumGen, scale*m_MixingStdDev[i]);
        proposed[i] = proposed[i] - floor( proposed[i] );
        proposed[i] = (proposed[i]*(range[1]-range[0]))+range[0];
      }
    }
  }

  return proposed;
}


double
ProposalDistribution
::Evaluate( std::vector< double > Theta1, std::vector< double > Theta2, double scale )
{
  // At the moment we are using a gaussian proposal distribution, so the scale is the standard deviation
  double probability;
  double exponent = 0, prefactor = 1;

  if ( m_SymmetricProposal ) {
    // If it's symmetric this doesn't matter
    probability = 1.0;
  } else {
    scale = (scale * (m_MaxScale - m_MinScale)) + m_MinScale;
    for ( int i = 0; i < Theta1.size(); i++ ) {
      exponent += -(Theta1[i]-Theta2[i])*(Theta1[i]-Theta2[i])/(2*scale*scale*m_MixingStdDev[i]*m_MixingStdDev[i]);
      prefactor = prefactor / (scale * sqrt( 2*M_PI ) );
    }
    probability = prefactor * exp( exponent );
  }
  return probability;
}

} // end namespace madai
