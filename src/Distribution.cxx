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

#include "MultiModel.h"

#include "MCMCRun.h"


namespace madai {

Distribution
::Distribution() :
  m_Model( NULL ),
  m_RandNumGen( NULL ),
  m_SepMap( false ),
  m_Timing( false ),
  m_Verbose( false ),
  m_Debug( false ),
  m_ParameterMap( NULL )
{
  const gsl_rng_type * rngtype;
  rngtype = gsl_rng_default;
  gsl_rng_env_setup();
  m_RandNumGen = gsl_rng_alloc( rngtype );
  gsl_rng_set( m_RandNumGen, time( NULL ) );
}


Distribution
::~Distribution()
{
}


double
Distribution
::Normal( double x, double mu, double sigma )
{
  return (1.0 / sqrt( 2 * M_PI * pow( sigma, 2 ) ) ) * exp( -pow( (x-mu) , 2 ) / ( 2 * pow( sigma,2 ) ) );
}


double
Distribution
::IntegratedNormal( double x, double mu, double sigma, double data_sigma )
{
  double likelihood=0;
  //cout << endl << "Inputs: x: " << x << " mu: " << mu << " sigma: " << sigma << " data_sigma: " << data_sigma << endl;
  // Three possible cases
  //The gaussians are identical:
  if ( (x == mu) && (sigma == data_sigma) ) {
    likelihood=1;
    //  cout << " Same gaussians: " << likelihood << endl;
  }

  //The gaussians are different but have the save variance:
  if ( (x != mu) && (sigma == data_sigma) ) {
    double point = (x+mu) / 2; //The mean, the only point the gaussians cross
    double right,left;
    if ( x < mu ) {
      left = x;
      right = mu;
    }
    if ( x > mu ) {
      left = mu;
      right = x;
    }
    likelihood = (2 + erf( (point-right) / (sqrt( 2 ) * sigma) ) - erf( (point-left) / (sqrt( 2 ) * sigma ) ) ) / 2;
    //  cout << " Same sigma: " << likelihood << endl;
  }
  //The gaussians are different and have different variances
  if ( sigma != data_sigma ) {
    //  cout << "two different gaussians " << endl;
    double point1, point2, left, right; //There can be two points where the gaussians cross
    point1 = (mu*sigma*sigma-x*data_sigma*data_sigma-sigma*data_sigma*sqrt((mu-x)*(mu-x)+2*(sigma-data_sigma)*log(sigma/data_sigma)))/(sigma*sigma-data_sigma*data_sigma);
    point2 = (mu*sigma*sigma-x*data_sigma*data_sigma+sigma*data_sigma*sqrt((mu-x)*(mu-x)+2*(sigma-data_sigma)*log(sigma/data_sigma)))/(sigma*sigma-data_sigma*data_sigma);
    //  cout << "point1: " << point1 << " point2: " << point2 << " numerator 1: " << mu*sigma*sigma-x*data_sigma*data_sigma << " numerator 2: " << sigma*data_sigma << " numerator " << (mu*sigma*sigma-x*data_sigma*data_sigma+sigma*data_sigma*sqrt((mu-x)*(mu-x)-2*sigma*log(data_sigma/sigma)+2*data_sigma*log(sigma/data_sigma))) << " denominator: " << sigma*sigma-data_sigma*data_sigma << endl;
    if ( point1 < point2 ) {
      left = point1;
      right = point2;
    }
    if ( point1 > point2 ) {
      left = point2;
      right = point1;
    }
    //  cout << "left: " << left << " right: " << right << " log(data_sigma/sigma) " << log(data_sigma/sigma) << " log(sigma/data_sigma) " << log(sigma/data_sigma) << " under the root " << (mu-x)*(mu-x)-2*sigma*log(data_sigma/sigma)+2*data_sigma*log(sigma/data_sigma) << endl;
    //There are three regions
    double x11=Normal( left-1, mu, sigma );
    double x12=Normal( (left+right)/2, mu, sigma );
    double x13=Normal( right+1, mu, sigma );
    double x21=Normal( left-1, x, data_sigma );
    double x22=Normal( (left+right)/2, x, data_sigma );
    double x23=Normal( right+1, x, data_sigma );
    //  cout << "x11 " << x11 << " x12 " << x12 << " x13 " << x13 << " x21 " << x21 << " x22 " << x22 << " x23 " << x23 << endl;
    //leftmost block
    if ( x11 > x21 ) {
      likelihood += ( 1 + erf( (left-x) / (sqrt(2) * data_sigma ) ) ) / 2;
      //    cout << "1 " << likelihood;
    }
    if ( x11 < x21 ) {
      likelihood += (1+erf((left-mu)/(sqrt(2)*sigma)))/2;
      //    cout << "1 " << likelihood;
    }
    //center block
    if ( x12 > x22 ) {
      likelihood += (erf((right-x)/(sqrt(2)*data_sigma))-erf((left-x)/(sqrt(2)*data_sigma)))/2;
      //    cout << " 2 " << likelihood;
    }
    if ( x12 < x22 ) {
      likelihood += (erf((right-mu)/(sqrt(2)*sigma))-erf((left-mu)/(sqrt(2)*sigma)))/2;
      //    cout << " 2 " << likelihood;
    }
    //rightmost block
    if ( x13 > x23 ) {
      likelihood += (1-erf((right-x)/(sqrt(2)*data_sigma)))/2;
      //    cout << " 3 " << likelihood << endl;
    }
    if ( x13 < x23 ) {
      likelihood += (1-erf((right-mu)/(sqrt(2)*sigma)))/2;
      //    cout << " 3 " << likelihood << endl;
    }
  }
  //cout << "the likelihood is: " << likelihood << endl;
  return likelihood;
}


double
Distribution
::Gaussian( double x, double mu, double sigma )
{
  return Normal( x, mu, sigma );
}


double
Distribution
::Gaussian( gsl_vector x, gsl_vector mu, gsl_matrix sigma )
{
  double out = 1;
  int n = x.size;
  for ( int i = 0; i < n; i++ ) {
    out = out*Normal(gsl_vector_get(&x,i),gsl_vector_get(&mu,i),gsl_matrix_get(&sigma,i,i));
  }
  //out=out/n;
  return out;
}


double
Distribution
::Gaussian( gsl_vector x, gsl_vector mu, gsl_matrix sigma, gsl_matrix data_sigma )
{
  double out = 1;
  int n = x.size;
  for ( int i = 0; i < n; i++ ) {
    out=out*IntegratedNormal(gsl_vector_get(&x,i),gsl_vector_get(&mu,i),gsl_matrix_get(&sigma,i,i),gsl_matrix_get(&data_sigma,i,i));
  }
  //out=out/n;
  return out;
}


double
Distribution
::Log_MVNormal( gsl_vector x, gsl_vector mu, gsl_matrix sigma )
{
  double OUT, det;
  int foobar, N;
  N = x.size;

  gsl_matrix * sigma_inv = gsl_matrix_calloc( N, N );
  gsl_matrix * tempsigma = gsl_matrix_alloc( N, N );
  gsl_permutation * p = gsl_permutation_alloc( N );
  gsl_vector * diff = gsl_vector_alloc( N );
  gsl_vector * temp = gsl_vector_alloc( N );

  gsl_vector_memcpy( diff, &x );
  gsl_matrix_memcpy( tempsigma, &sigma );
  gsl_vector_sub( diff, &mu );

  //invert matrix using LU decomposition
  gsl_linalg_LU_decomp( tempsigma, p, &foobar );
  gsl_linalg_LU_invert( tempsigma, p, sigma_inv );

  det = gsl_linalg_LU_det( tempsigma, foobar );

  //multiply matrix and left vector together using CBLAS routines
  gsl_blas_dsymv( CblasUpper, 1.0, sigma_inv, diff, 0.0, temp );

  gsl_blas_ddot( diff, temp, &OUT );

  //OUT = (-1.0/2.0)*OUT - double(N/2)*log(2*M_PI) - (0.5)*log(det);
  OUT = (-1.0/2.0) * OUT;

  gsl_matrix_free( sigma_inv );
  gsl_permutation_free( p );
  gsl_vector_free( diff );
  gsl_vector_free( temp );

  return OUT;
}


double
Distribution
::MVNormal( gsl_vector x, gsl_vector mu, gsl_matrix sigma )
{
  double like = Log_MVNormal( x, mu, sigma );
  return exp( Log_MVNormal( x, mu, sigma ) );
}


double
Distribution
::LogNormal( double x, double mu, double sigma )
{
  double OUT = (1.0/(x*sigma))*(1.0/sqrt(2.0*M_PI))*exp((-1.0/(2.0*pow(sigma,2)))*(pow((log(x)-mu), 2)));
  return OUT;
}

} // end namespace madai
