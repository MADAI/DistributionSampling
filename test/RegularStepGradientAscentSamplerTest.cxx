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

#include <cstdlib>
#include <iostream>
#include <cmath>

#include "Gaussian2DModel.h"
#include "RegularStepGradientAscentSampler.h"
#include "Trace.h"


int main(int argc, char *argv[])
{
  madai::Gaussian2DModel model;

  madai::RegularStepGradientAscentSampler sampler;
  sampler.SetModel( &model );
  sampler.Maximize(); // We want to maximize the likelihood function

  // Set the step size.
  double stepSize = 2.0;
  sampler.SetStepSize( stepSize );

  // Set initial parameter values.
  sampler.SetParameterValue( "X", 21.0 );
  sampler.SetParameterValue( "Y", -13.5 );

  std::vector< double > currentParameters;
  for (unsigned int i = 0; i < 1000; i++) {
    currentParameters = sampler.GetCurrentParameters();
    madai::Sample sample = sampler.NextSample();
  }

  // Adjust the step size up by a factor of 10
  stepSize = 2000.0;
  sampler.SetStepSize( stepSize );

  for (unsigned int i = 0; i < 2000; i++) {
    currentParameters = sampler.GetCurrentParameters();
    madai::Sample sample = sampler.NextSample();
  }

  // Adjust the step size up by a factor of 10
  stepSize = 400000.0;
  sampler.SetStepSize( stepSize );

  for (unsigned int i = 0; i < 3000; i++) {
    currentParameters = sampler.GetCurrentParameters();
    madai::Sample sample = sampler.NextSample();
  }

  double modelMeanX;
  double modelMeanY;
  model.GetMeans( modelMeanX, modelMeanY );

  if ( std::abs( modelMeanX - currentParameters[0] ) > 1.0e-2 ||
       std::abs( modelMeanY - currentParameters[1] ) > 1.0e-2 ) {
    std::cerr << "RegularStepGradientAscentSampler failed to converge "
              << "on the expected solution." << std::endl;
    std::cerr << "Expected currentParameters to be (" << modelMeanX << ", "
              << modelMeanY << "), got (" << currentParameters[0] << ", "
              << currentParameters[1] << ") instead." << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
