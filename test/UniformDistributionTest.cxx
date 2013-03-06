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

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>

#include "UniformDistribution.h"


int main( int argc, char* argv[] )
{
  madai::UniformDistribution distribution;
  if ( distribution.GetMinimum() != 0.0 ) {
    std::cerr << "UniformDistribution minimum should be 0.0" << std::endl;
    return EXIT_FAILURE;
  }

  if ( distribution.GetMaximum() != 1.0 ) {
    std::cerr << "UniformDistribution maximum should be 1.0" << std::endl;
    return EXIT_FAILURE;
  }

  double newMinimum = -2.0;
  distribution.SetMinimum( newMinimum );
  if ( distribution.GetMinimum() != newMinimum ) {
    std::cerr << "UniformDistribution::GetMinimum() expected to be " << newMinimum
	      << ", was " << distribution.GetMinimum() << std::endl;
    return EXIT_FAILURE;
  }

  double newMaximum = 4.2;
  distribution.SetMaximum( newMaximum );
  if ( distribution.GetMaximum() != newMaximum ) {
    std::cerr << "UniformDistribution::GetMaximum() expected to be " << newMaximum
	      << ", was " << distribution.GetMaximum() << std::endl;
    return EXIT_FAILURE;
  }

  if ( distribution.GetLogProbabilityDensity( newMinimum - 1.0 ) !=
       -std::numeric_limits< double >::infinity() ) {
    std::cerr << "UniformDistribution::GetLogProbabilityDensity() for value less than "
	      << "minimum was not -infinity." << std::endl;
    return EXIT_FAILURE;
  }

  if ( distribution.GetLogProbabilityDensity( newMaximum + 1.0 ) !=
       -std::numeric_limits< double >::infinity() ) {
    std::cerr << "UniformDistribution::GetLogProbabilityDensity() for value greater than "
	      << "maximum was not -infinity." << std::endl;
    return EXIT_FAILURE;
  }

  double expectedLog = -log( newMaximum - newMinimum );
  double actualLog = distribution.GetLogProbabilityDensity( 0.5 * ( newMinimum + newMaximum ) );
  if ( actualLog != expectedLog ) {
    std::cerr << "UniformDistribution::GetLogProbabilityDensity() for value in range expected "
	      << "to return " << expectedLog << ", got " << actualLog << " instead."
	      << std::endl;
    return EXIT_FAILURE;
  }

  if ( distribution.GetProbabilityDensity( newMinimum - 1.0 ) != 0.0 ) {
    std::cerr << "UniformDistribution::GetProbabilityDensity() for value less than minimum "
	      << "should have been 0.0, got << "
	      << distribution.GetProbabilityDensity( newMinimum ) << " instead." << std::endl;
    return EXIT_FAILURE;
  }

  if ( distribution.GetProbabilityDensity( newMaximum + 1.0 ) != 0.0 ) {
    std::cerr << "UniformDistribution::GetProbabilityDensity() for value greater than maximum "
	      << "should have been 0.0, got "
	      << distribution.GetProbabilityDensity( newMaximum ) << " instead." << std::endl;
    return EXIT_FAILURE;
  }

  double expectedDensity = 1.0 / ( newMaximum - newMinimum );
  double actualDensity = distribution.GetProbabilityDensity( 0.5 * ( newMinimum + newMaximum ) );
  if ( expectedDensity != actualDensity ) {
    std::cerr << "UniformDistribution::GetProbabilityDensity() for value in range should "
	      << "have been " << expectedDensity << ", got " << actualDensity
	      << "instead." << std::endl;
    return EXIT_FAILURE;
  }

  double x = 0.32;
  double expectedPercentile = x * ( newMaximum - newMinimum ) + newMinimum;
  double actualPercentile = distribution.GetPercentile( x );
  if ( expectedPercentile != actualPercentile ) {
    std::cerr << "UniformDistribution::GetPercentile() for value " << x << " should have "
	      << "been " << expectedPercentile << ", got " << actualPercentile << std::endl;
    return EXIT_FAILURE;
  }

}
