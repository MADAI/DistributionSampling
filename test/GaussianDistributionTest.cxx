/*=========================================================================
 *
 *  Copyright 2011-2013 The University of North Carolina at Chapel Hill
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

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>

#include "GaussianDistribution.h"


int main( int argc, char* argv[] )
{
  madai::GaussianDistribution distribution;
  
  if ( distribution.GetMean() != 0.0 ) {
    std::cerr << "Default mean is not 0.0" << std::endl;
    return EXIT_FAILURE;
  }

  if ( distribution.GetStandardDeviation() != 1.0 ) {
    std::cerr << "Default standard deviation is not 1.0" << std::endl;
    return EXIT_FAILURE;
  }

  double newMean = 3.2;
  distribution.SetMean( newMean );
  if ( distribution.GetMean() != newMean ) {
    std::cerr << "GaussianDistribution::GetMean() expected to be " << newMean << ", was "
        << distribution.GetMean() << " instead." << std::endl;
    return EXIT_FAILURE;
  }

  double newStandardDeviation = 2.3;
  distribution.SetStandardDeviation( newStandardDeviation );
  if ( distribution.GetStandardDeviation() != newStandardDeviation ) {
    std::cerr << "GaussianDistribution::GetStandardDeviation() expected to be "
        << newStandardDeviation << ", was " << distribution.GetStandardDeviation()
        << " instead." << std::endl;
    return EXIT_FAILURE;
  }

  double x = 2.9;
  double variance = newStandardDeviation * newStandardDeviation;
  double normalization = 1.0 / sqrt( 2.0 * M_PI * variance );
  double expectedLogDensity = log( normalization ) + 
    -( x - newMean ) * ( x - newMean ) / ( 2.0 * variance );
  double actualLogDensity = distribution.GetLogProbabilityDensity( x );
  if ( expectedLogDensity != actualLogDensity ) {
    std::cerr << "GaussianDistribution::GetLogProbabilityDensity() expected to return "
        << expectedLogDensity << ", got " << actualLogDensity << " instead."
        << std::endl;
    return EXIT_FAILURE;
  }

  double expectedDensity = normalization *
    exp( -( x - newMean ) * ( x - newMean ) / ( 2.0 * variance ) );
  double actualDensity = distribution.GetProbabilityDensity( x );
  if ( fabs( expectedDensity - actualDensity ) > 1e-4 ) {
    std::cerr << "GaussianDistribution::GetProbabilityDensity() expected to return "
        << expectedDensity << ", got " << actualDensity << " instead." << std::endl;
    return EXIT_FAILURE;
  }

  double expectedPercentile = -0.67448 * newStandardDeviation + newMean;
  double actualPercentile = distribution.GetPercentile( 0.25 );
  if ( fabs( actualPercentile - expectedPercentile ) > 1e-4 ) {
    std::cerr << "GaussianDistribution::GetPercentile( 0.25 ) expected to return "
        << expectedPercentile << ", got " << actualPercentile << " instead." << std::endl;
    return EXIT_FAILURE;
  }

  expectedPercentile = 0.67448 * newStandardDeviation + newMean;
  actualPercentile = distribution.GetPercentile( 0.75 );
  if ( fabs( actualPercentile - expectedPercentile ) > 1e-4 ) {
    std::cerr << "GaussianDistribution::GetPercentile( 0.75 ) expected to return "
        << expectedPercentile << ", got " << actualPercentile << " instead." << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
