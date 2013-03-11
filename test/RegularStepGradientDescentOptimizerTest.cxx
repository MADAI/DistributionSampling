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
#include "RegularStepGradientDescentSampler.h"
#include "Trace.h"


int main(int argc, char *argv[])
{
  madai::Gaussian2DModel *model =
    new madai::Gaussian2DModel();
  model->LoadConfigurationFile( "file.txt" ); // TODO - does nothing

  madai::RegularStepGradientDescentSampler *sampler =
    new madai::RegularStepGradientDescentSampler();
  sampler->SetModel( model );
  sampler->MinimizeOn(); // We want to minimize this function

  //madai::Trace *trace = new madai::Trace();
  madai::Trace trace;

  // Set the step size.
  double stepSize = 20.0;
  sampler->SetStepSize( stepSize );

  // // Pick which output scalar to optimize.
  // sampler->SetOutputScalarToOptimize( "Value " );

  // Set initial parameter values.
  sampler->SetParameterValue( "X", 21.0 );
  sampler->SetParameterValue( "Y", -13.5 );

  std::vector< double > currentParameters;
  for (unsigned int i = 0; i < 50; i++) {
    currentParameters = sampler->GetCurrentParameters();
    madai::Sample sample = sampler->NextSample();
    trace.Add( sample );
  }

  double modelMeanX;
  double modelMeanY;
  model->GetMeans( modelMeanX, modelMeanY );

  if ( std::abs( modelMeanX - currentParameters[0] ) > 1.0e-3 ||
       std::abs( modelMeanY - currentParameters[1] ) > 1.0e-3 ) {
    std::cerr << "RegularStepGradientDescentSampler failed to converge "
              << "on the expected solution." << std::endl;
    std::cerr << "Expected currentParameters to be (" << modelMeanX << ", "
              << modelMeanY << "), got (" << currentParameters[0] << ", "
              << currentParameters[1] << ") instead." << std::endl;
    return EXIT_FAILURE;
  }

  //delete trace;

  return EXIT_SUCCESS;
}
