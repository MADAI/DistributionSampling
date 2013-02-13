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

#include "CosmoLikelihoodDistribution.h"

#include <time.h>

namespace madai {

CosmoLikelihoodDistribution
::CosmoLikelihoodDistribution( Model *in_Model )
{
  m_Model = in_Model;
  m_SepMap = parameter::getB( m_Model->m_ParameterMap, "LIKELIHOOD_PARAMETER_MAP", false );

  if ( m_SepMap ) {
    std::string parmapfile = m_Model->m_DirectoryName + "defaultpars/likelihood.param";
    m_ParameterMap = new parameterMap;
    parameter::ReadParsFromFile( *m_ParameterMap, parmapfile );
  } else {
    m_ParameterMap = &(m_Model->m_ParameterMap);
  }

  // cout << "Like param map made." << endl;

  m_Timing = parameter::getB( *m_ParameterMap, "TIMING", false) || parameter::getB(*m_ParameterMap, "TIME_LIKELIHOOD", false );
  m_Verbose = parameter::getB( *m_ParameterMap, "VERBOSE", false) || parameter::getB(*m_ParameterMap, "VERBOSE_LIKELIHOOD", false );

  m_Data = GetData();
  m_IntData.resize( m_Data.size() );
  for ( int i = 1; i < m_IntData.size(); i++ ) {
    m_IntData[i] = gsl_ran_poisson( m_RandNumGen, m_Data[i] );
  }
}


CosmoLikelihoodDistribution
::~CosmoLikelihoodDistribution()
{
  //delete emulator;
}


double
CosmoLikelihoodDistribution
::Evaluate( std::vector<double> ModelMeans,
            std::vector<double> ModelErrors )
{
  clock_t begintime;
  double likelihood = 0.0,dll;

  if ( m_Timing ) {
    begintime = clock();
  }

  for ( int i = 1; i < ModelMeans.size(); i++ ) {
    dll = log( gsl_ran_poisson_pdf( m_IntData[i],ModelMeans[i] ) );
    //printf("ModelMeans[%d]=%g, intDATA[%d]=%d, Dloglikelihood=%g\n",i,ModelMeans[i],i,intDATA[i],dll);
    //likelihood += log(gsl_ran_poisson_pdf(static_cast<unsigned int>(ModelMeans[i] + 0.5), DATA[i]));
    likelihood += dll;
  }
  printf( "XXXXX LogLikelihood=%g XXXXXX\b", likelihood );
  if ( likelihood > -0.0001 ) {
    for ( int i = 1; i < ModelMeans.size(); i++ ) {
      dll = log( gsl_ran_poisson_pdf( m_IntData[i], ModelMeans[i] ) );
      printf( "ModelMeans[%d]=%g, intDATA[%d]=%d, Dloglikelihood=%g\n", i, ModelMeans[i], i, m_IntData[i], dll );
      //likelihood += log(gsl_ran_poisson_pdf(static_cast<unsigned int>(ModelMeans[i] + 0.5), DATA[i]));
    }
    exit( 1 );
  }

  if ( !(m_Model->m_LogLike) ) {
    likelihood = exp( likelihood );
  }

  if ( m_Timing ) {
    std::cout << "Likelihood evaluation took " << (clock()-begintime)*1000/CLOCKS_PER_SEC << " ms." << std::endl;
  }

  return likelihood;
}


std::vector< double >
CosmoLikelihoodDistribution
::GetData()
{
  std::vector< double > datameans;
  std::stringstream ss;
  parameterMap actualparmap;
  std::ifstream inputfile;

  std::string actual_filename = m_Model->m_DirectoryName + "/defaultpars/actual.param";
  parameter::ReadParsFromFile( actualparmap, actual_filename );

  std::vector< std::string > temp_names = parameter::getVS( actualparmap, "NAMES", "" );
  std::vector< double > temp_values = parameter::getV( actualparmap, "VALUES", "" );

  std::vector< double > ActualParamValues;
  std::vector< std::string > ActualParamNames;
  ActualParamValues = temp_values;
  ActualParamNames = temp_names;

  ss << "cosmosurvey";
  for ( int i = 0; i < temp_names.size(); i++ ) {
    ss << " -" << temp_names[i] << " " << temp_values[i];
  }
  ss << " -nz 10 -nf 10 -ob .0406 > data_output.dat" << endl;

  std::cout << ss.str() << std::endl;
  std::cout << "Waiting on cosmosurvey...";
  std::cout.flush();
  int result = system( (ss.str()).c_str() );
  std::cout << "Done." << std::endl;

  inputfile.open( "data_output.dat" );

  int counter = 0;
  while ( !inputfile.eof() ) {
    counter++;
    double temp;
    inputfile >> temp;
    std::cout << "Data point " << counter << ": " << temp << std::endl;
    datameans.push_back( temp );
  }

  return datameans;
}

} // end namespace madai
