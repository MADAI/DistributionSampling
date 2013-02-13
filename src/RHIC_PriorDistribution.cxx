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

#include "RHIC_PriorDistribution.h"

#include "Model.h"


namespace madai {


RHIC_PriorDistribution
::RHIC_PriorDistribution( Model * in_Model )
{
  m_Model = in_Model;
  m_SepMap = parameter::getB( m_Model->m_ParameterMap, "PRIOR_PARAMETER_MAP", false );

  if ( m_SepMap ) {
    std::string parmapfile = m_Model->m_DirectoryName + "/defaultpars/prior.param";
    m_ParameterMap = new parameterMap;
    parameter::ReadParsFromFile( *m_ParameterMap, parmapfile );
    //parameter::ReadParsFromFile( *parmap, parameter_file_name );
  } else {
    m_ParameterMap = &(m_Model->m_ParameterMap);
  }

  m_Prior  = parameter::getS( *m_ParameterMap,"PRIOR","UNIFORM" ); // Specify what type of prior function to use: UNIFORM, GAUSSIAN, STEP. //I need to add "MIXED"
  m_Scaled = parameter::getB( *m_ParameterMap,"SCALED", true ); //Specifies wethere the values are given 0 to 1, or min_val to max_val

  if ( std::strcmp( m_Prior.c_str(), "GAUSSIAN") == 0 ) {
    // Read in parameters for Gaussians
    m_GaussianMeans = parameter::getV( *m_ParameterMap, "GAUSSIAN_MEANS", "" );
    m_GaussianSTDVS = parameter::getV( *m_ParameterMap, "GAUSSIAN_STDVS", "" );
    if ( m_GaussianMeans.size() == 0 || m_GaussianSTDVS.size() == 0 ) {
      std::cout << "Error in prior_rhic.cc: RHIC_PriorDistribution::RHIC_PriorDistribution(MCMCConfiguration)" << std::endl;
      std::cout << "GAUSSIAN_MEANS or GAUSSIAN_STDVS not specified. Exiting" << std::endl;
      exit( 1 );
    }
    if ( m_GaussianMeans.size() != m_GaussianSTDVS.size() ) {
      std::cout << "Error in prior_rhic.cc: RHIC_PriorDistribution::RHIC_PriorDistribution(MCMCConfiguration)" << std::endl;
      std::cout << "Length of GAUSSIAN_MEANS and GAUSSIAN_STDVS are not the same." << std::endl;
      std::cout << "Lenght of GAUSSIAN_MEANS = " << m_GaussianMeans.size() << std::endl;
      std::cout << "Lenght of GAUSSIAN_STDVS = " << m_GaussianSTDVS.size() << std::endl;
      std::cout << "Exiting" << std::endl;
      exit(1);
    }
  }
  if ( std::strcmp( m_Prior.c_str(), "STEP" ) == 0 ) {
    /* JFN 9/24/12 9:20am- Just a note. At the moment I am coding a step function prior because Scott asked me to. But this is
       both a) going to cause runtime errors, and b) the wrong way to do this. If we have a step function prior, that should be
       actualized by adjusting the ranges.*/
    // Read in parameters for step functions
    m_StepMeans = parameter::getV( *m_ParameterMap, "STEP_MEANS", "" );
    m_StepSide  = parameter::getVS( *m_ParameterMap, "STEP_SIDE", "" );
    if ( m_StepMeans.size() == 0 || m_StepSide.size() == 0 ) {
      std::cout << "Error in prior_rhic.cc: RHIC_PriorDistribution::RHIC_PriorDistribution(MCMCConfiguration)" << std::endl;
      std::cout << "STEP_MEANS or STEP_SIDE not specified. Exiting" << std::endl;
      exit( 1 );
    }
    if ( m_StepMeans.size() != m_StepSide.size() ) {
      std::cout << "Error in prior_rhic.cc: RHIC_PriorDistribution::RHIC_PriorDistribution(MCMCConfiguration)" << std::endl;
      std::cout << "Length of STEP_MEANS and STEP_SIDE are not the same." << std::endl;
      std::cout << "Lenght of STEP_MEANS = " << m_StepMeans.size() << std::endl;
      std::cout << "Lenght of STEP_SIDE = " << m_StepSide.size() << std::endl;
      std::cout << "Exiting" << std::endl;
      exit( 1 );
    }
  }
}


double
RHIC_PriorDistribution
::Evaluate( std::vector< double > Theta ) {
  /*double mean = parameter::getD( *parmap, "PRIOR_MEAN", -3.7372 );
  double sigma = parameter::getD( *parmap, "PRIOR_SIGMA", 1.6845 );
  return Normal( log( Theta.GetValue( "SIGMA" ) ), mean, sigma );*/
  if ( std::strcmp( m_Prior.c_str(), "UNIFORM" ) == 0 ) {
    // If the prior is uniform, it doesn't matter what value we return as long as it is consistent
    return 1.0;
  }
  if ( std::strcmp( m_Prior.c_str(), "GAUSSIAN") == 0 ) {
    // The return value needs to be caluclated form a multivariate gaussian
    int N = Theta.size();
    gsl_matrix * sigma = gsl_matrix_calloc( N, N );
    gsl_vector * theta = gsl_vector_alloc( N );
    gsl_vector * means = gsl_vector_alloc( N );
    for ( int i = 0; i < N; i++ ) {
      gsl_matrix_set( sigma, i, i, m_GaussianSTDVS[i] );
      gsl_vector_set( theta, i, Theta[i] );
      gsl_vector_set( means, i, m_GaussianMeans[i] );
    }
    return MVNormal( *theta, *means, *sigma );
  }
  if ( std::strcmp( m_Prior.c_str(), "STEP") == 0 ) {
    // The return value is either 0, or an arbitary consistent value
    for ( int i = 0; i < Theta.size(); i++ ) {
      // At the moment, scaled vs unscaled doesn't make any difference. In both they are treated as scaled. To be fixed
      if ( m_Scaled ) {
        if ( ( (Theta[i] > m_StepMeans[i]) && ( std::strcmp( m_StepSide[i].c_str(), "LOW" ) ) ) ||
             ( (Theta[i] < m_StepMeans[i]) && ( strcmp( m_StepSide[i].c_str(), "HIGH" ) ) ) ) {
          return 0; //... this seems like it could cause problems
        }
      }
      if ( !m_Scaled ) {
        if ( ( (Theta[i] > m_StepMeans[i]) && ( std::strcmp(m_StepSide[i].c_str(),"LOW"))) ||
             ( (Theta[i] < m_StepMeans[i]) && ( std::strcmp(m_StepSide[i].c_str(),"HIGH") ) ) ) {
          return 0; //... this seems like it could cause problems
        }
      }
    }
    return 1.0; // if the thetas have survived all of the step function checks
  }
}

} // end namespace madai
