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

#include "TestLikelihoodDistribution.h"

#include <time.h>


namespace madai {


TestLikelihoodDistribution
::TestLikelihoodDistribution( Model *in_Model )
{
  m_Model = in_Model;
  m_SepMap = parameter::getB( m_Model->m_ParameterMap, "LIKELIHOOD_PARAMETER_MAP", false );

  if ( m_SepMap ) {
    std::string parmapfile = m_Model->m_DirectoryName + "/defaultpars/likelihood.param";
    m_ParameterMap = new parameterMap;
    parameter::ReadParsFromFile( *m_ParameterMap, parmapfile );
  } else {
    m_ParameterMap = &(m_Model->m_ParameterMap);
  }

  // cout << "Like param map made." << endl;

  m_Timing  = parameter::getB( *m_ParameterMap, "TIMING", false) || parameter::getB(*m_ParameterMap, "TIME_LIKELIHOOD", false );
  m_Verbose = parameter::getB( *m_ParameterMap, "VERBOSE", false) || parameter::getB(*m_ParameterMap, "VERBOSE_LIKELIHOOD", false );

  // cout << "params declared." << endl;

  m_Data = GetData();

  //testing the outputs of the emulator at various points //
  m_EmulatorTest.open( "PCA0.dat" );
  m_EmulatorTest.close();
}


TestLikelihoodDistribution
::~TestLikelihoodDistribution()
{
}


double
TestLikelihoodDistribution
::Evaluate( std::vector< double > ModelMeans, std::vector< double > ModelErrors )
{
  clock_t begintime;
  double likelihood;

  if ( m_Timing ) {
    begintime = clock();
  }

  //Initialize GSL containers
  int N = ModelErrors.size();
  std::cerr << "Length of ModelErrors = " << N << std::endl;
  //gsl_matrix * sigma = gsl_matrix_calloc( N, N );
  gsl_vector * model = gsl_vector_alloc( N );
  gsl_vector * mu = gsl_vector_alloc( N );

  //Read in appropriate elements
  for ( int i = 0; i < N; i++ ) {
    // gsl_matrix_set( sigma, i, i, Theta.GetValue("SIGMA") );
    //gsl_matrix_set( sigma, i, i, ModelErrors[i] );
    gsl_vector_set( model, i, ModelMeans[i] );
    gsl_vector_set( mu, i, m_Data[i] );
  }

  //likelihood = Log_MVNormal(*model, *mu, *sigma);

  for ( int i = 0; i < 4; i++ ) {
    likelihood += (ModelMeans[i]-m_Data[i])*(ModelMeans[i]-m_Data[i])/2; //This is a gaussian, but it would be "log(exp(stuff))", so I just left it "stuff"
  }

  if ( !(m_Model->m_LogLike) ) {
    likelihood = exp( likelihood );
  }

  if ( m_Verbose ) {
    /*double sum = 0.0;

    for(int i = 0; i< N; i++){
      sum += (gsl_vector_get(model, i) - gsl_vector_get(mu, i));
    }
    sum = sum/(double)N;
    cout << "Average difference between outputs:" << sum << endl;*/
  }

  //deallocate GSL containers.
  gsl_vector_free( model );
  gsl_vector_free( mu );
  //gsl_matrix_free( sigma );

  if ( m_Timing ) {
    std::cout << "Likelihood evaluation took " << (clock()-begintime)*1000/CLOCKS_PER_SEC << " ms." << std::endl;
  }

  std::cout << "PCA 0: " << ModelMeans[0] << std::endl;

  m_EmulatorTest.open( "PCA0.dat", ios_base::app );
  m_EmulatorTest << ModelMeans[0] << std::endl;
  m_EmulatorTest.close();
  // emulator_test << ModelMeans[0] << endl;

  return likelihood;
}


std::vector< double >
TestLikelihoodDistribution
::GetData()
{
  //Four separate gaussians with means of 1, 2, 3, and 4 and standard deviations of 2. The means will be the input,
  //and three values on the gaussian will be the output.
  std::vector< double > datameans;
  int mymeans[] = {1,2,3,4};
  datameans.assign( mymeans, mymeans + 4 );
  //vector<double> dataerror;
  /*
  parameterMap actualparmap;

  string actual_filename = mcmc->parameterfile + "/actual.param";
  parameter::ReadParsFromFile( actualparmap, actual_filename );

  std::vector<string> temp_names  = parameter::getVS( actualparmap, "NAMES", "");
  std::vector<double> temp_values = parameter::getV( actualparmap, "VALUES", "");

  ParameterSet ActualParams;
  ActualParams.Initialize(temp_names, temp_values);

  emulator->QueryEmulator(ActualParams, datameans, dataerror);*/

  return datameans;
}

} // end namespace madai
