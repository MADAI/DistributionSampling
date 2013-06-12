/*=========================================================================
 *
 *  Copyright 2011-2013 The University of North Carolina at Chapel Hill
 *  All rights reserved.
 *
 *  Licensed under the MADAI Software License. You may obtain a copy of
 *  this license at
 *
 *         https://madai-public.cs.unc.edu/visualization/software-license/
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

#include "Gaussian2DModel.h"
#include "PercentileGridSampler.h"


int main( int, char *[] )
{
  madai::Gaussian2DModel model;

  unsigned int numberOfSamples = 10;
  madai::PercentileGridSampler sampler;
  sampler.SetModel( &model );
  sampler.SetNumberOfSamples( numberOfSamples );

  // Check that the number of samples is bumped up to 16
  unsigned int expectedNumberOfSamples =
    std::pow( std::ceil( std::sqrt( numberOfSamples ) ), 2.0 );
  unsigned int actualNumberOfSamples = sampler.GetNumberOfSamples();
  if ( expectedNumberOfSamples != actualNumberOfSamples ) {
    std::cerr << "Number of samples expected to be " << expectedNumberOfSamples
              << " but got " << actualNumberOfSamples << "\n";
    return EXIT_FAILURE;
  }

  // Check that parameter activation/deactivation results in the right
  // number of samples.
  sampler.DeactivateParameter( "X" );
  expectedNumberOfSamples =
    static_cast< unsigned int >( std::sqrt( expectedNumberOfSamples ) );

  actualNumberOfSamples = sampler.GetNumberOfSamples();
  if ( expectedNumberOfSamples != actualNumberOfSamples ) {
    std::cerr << "When X is deactivated, number of samples was expected to be "
              << expectedNumberOfSamples << " but got "
              << actualNumberOfSamples << "\n";
    return EXIT_FAILURE;
  }

  // Generate one sample and see if it is at the right place
  double x = 23.2;
  double y = -14.0;
  if ( sampler.SetParameterValue( "X", x ) != madai::Sampler::NO_ERROR ) {
    std::cerr << "Could not set parameter value for X\n";
    return EXIT_FAILURE;
  }

  for ( unsigned int i = 0; i < sampler.GetNumberOfSamples(); ++i ) {
    madai::Sample sample = sampler.NextSample();

    if ( sample.m_ParameterValues[0] != x ) {
      std::cerr << "Expected sample " << i << " parameter to be (" << x
                << ") but got (" << sample.m_ParameterValues[0] << ").\n";
      return EXIT_FAILURE;
    }
  }

  // Reset the sampler
  sampler.Reset();

  // Now turn off Y and turn X back on
  sampler.ActivateParameter( "X" );
  sampler.DeactivateParameter( "Y" );

  if ( sampler.SetParameterValue( "Y", y ) != madai::Sampler::NO_ERROR ) {
    std::cerr << "Could not set parameter value for Y\n";
    return EXIT_FAILURE;
  }

  for ( unsigned int i = 0; i < sampler.GetNumberOfSamples(); ++i ) {
    madai::Sample sample = sampler.NextSample();

    if ( sample.m_ParameterValues[1] != y ) {
      std::cerr << "Expected sample " << i << " parameter to be (" << y
                << ") but got (" << sample.m_ParameterValues[1] << ").\n";
      return EXIT_FAILURE;
    }
  }

  actualNumberOfSamples = sampler.GetNumberOfSamples();
  if ( expectedNumberOfSamples != actualNumberOfSamples ) {
    std::cerr << "When Y is deactivated, number of samples was expected to be "
              << expectedNumberOfSamples << " but got "
              << actualNumberOfSamples << "\n";
    return EXIT_FAILURE;
  }

  // Deactivate X
  sampler.DeactivateParameter( "X" );
  sampler.Reset();

  expectedNumberOfSamples = 0;
  actualNumberOfSamples = sampler.GetNumberOfSamples();

  if ( expectedNumberOfSamples != actualNumberOfSamples ) {
    std::cerr << "Expected " << expectedNumberOfSamples << " samples when X and Y are "
              << "deactivated but got " << actualNumberOfSamples << "\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
