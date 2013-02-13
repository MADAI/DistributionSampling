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

#include "InterpolatorPriorDistribution.h"


namespace madai {

InterpolatorPriorDistribution
::InterpolatorPriorDistribution( Model * in_Model ) :
  m_Model( in_Model ),
  m_Scaled( false )
{
  m_Model = in_Model;
  m_SepMap = parameter::getB( m_Model->m_ParameterMap, "PRIOR_PARAMETER_MAP", false );

  if ( m_SepMap ) {
    std::string parmapfile = m_Model->m_DirectoryName + "/defaultpars/prior.param";
    m_ParameterMap = new parameterMap;
    parameter::ReadParsFromFile( *m_ParameterMap, parmapfile );
  //parameter::ReadParsFromFile( *parmap, parameter_file_name );
  } else {
    m_ParameterMap= &(m_Model->m_ParameterMap);
  }

  m_Prior  = parameter::getS( *m_ParameterMap,"PRIOR","UNIFORM" ); // Specify what type of prior function to use: UNIFORM, GAUSSIAN, STEP. //I need to add "MIXED"
  m_Scaled = parameter::getB( *m_ParameterMap,"SCALED",true ); //Specifies wethere the values are given 0 to 1, or min_val to max_val

  if ( strcmp(m_Prior.c_str(),"GAUSSIAN") == 0 ) {
    // Read in parameters for Gaussians
    m_GaussianMeans = parameter::getV( *m_ParameterMap, "GAUSSIAN_MEANS","" );
    m_GaussianSTDVS = parameter::getV( *m_ParameterMap, "GAUSSIAN_STDVS","" );
    if ( m_GaussianMeans.size() == 0 || m_GaussianSTDVS.size() == 0 ) {
      std::cout << "Error in prior_Interpolator.cc: InterpolatorPriorDistribution::InterpolatorPriorDistribution(MCMC)" << std::endl;
      std::cout << "GAUSSIAN_MEANS or GAUSSIAN_STDVS not specified. Exiting" << std::endl;
      exit( 1 );
    }
    if ( m_GaussianMeans.size() != m_GaussianSTDVS.size() ) {
      std::cout << "Error in prior_Interpolator.cc: InterpolatorPriorDistribution::InterpolatorPriorDistribution(MCMC)" << std::endl;
      std::cout << "Length of GAUSSIAN_MEANS and GAUSSIAN_STDVS are not the same." << std::endl;
      std::cout << "Lenght of GAUSSIAN_MEANS = " << m_GaussianMeans.size() << std::endl;
      std::cout << "Lenght of GAUSSIAN_STDVS = " << m_GaussianSTDVS.size() << std::endl;
      std::cout << "Exiting" << std::endl;
      exit( 1 );
    }
  }
}


double
InterpolatorPriorDistribution
::Evaluate( std::vector< double > Theta )
{
  /*double mean = parameter::getD( *parmap, "PRIOR_MEAN", -3.7372 );
  double sigma = parameter::getD( *parmap, "PRIOR_SIGMA", 1.6845 );
  return Normal( log( Theta.GetValue("SIGMA")), mean, sigma );*/
  if ( strcmp( m_Prior.c_str(), "UNIFROM") == 0 ) {
    // If the prior is uniform, it doesn't matter what value we return as long as it is consistent
    return 1.0;
  }
  if ( strcmp( m_Prior.c_str(), "GAUSSIAN") == 0 ) {
    // The return value needs to be caluclated form a multivariate gaussian
    int N = Theta.size();
    gsl_matrix * sigma = gsl_matrix_calloc( N, N );
    gsl_vector * theta = gsl_vector_alloc( N );
    gsl_vector * means = gsl_vector_alloc( N );
    for( int i = 0; i < N; i++ ) {
      gsl_matrix_set( sigma, i, i, m_GaussianSTDVS[i] );
      gsl_vector_set( theta, i, Theta[i] );
      gsl_vector_set( means, i, m_GaussianMeans[i] );
    }
    return MVNormal( *theta,*means,*sigma );
  }
}

} // end namespace madai
