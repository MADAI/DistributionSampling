/*=========================================================================
 *
 *  Copyright (c) 2010-2012 The University of North Carolina at Chapel Hill
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

#include "Gaussian2DModel.h"
#include "RegularStepGradientDescentOptimizer.h"
#include "Trace.h"


int main(int argc, char *argv[])
{
  madai::Gaussian2DModel *model =
    new madai::Gaussian2DModel();
  model->LoadConfigurationFile( "file.txt" ); // TODO - does nothing

  madai::RegularStepGradientDescentOptimizer *optimizer =
    new madai::RegularStepGradientDescentOptimizer( model );
  optimizer->MinimizeOff(); // We want to maximize this function

  madai::Trace *trace = new madai::Trace();

  // Set the step size.
  double stepSize = 20.0;
  optimizer->SetStepSize( stepSize );

  // Pick which output scalar to optimize.
  optimizer->SetOutputScalarToOptimize( "Value " );

  // Set initial parameter values.
  optimizer->SetParameterValue( "X", 21.0 );
  optimizer->SetParameterValue( "Y", -13.5 );

  std::vector< double > currentParameters;
  for (unsigned int i = 0; i < 50; i++) {
    currentParameters = optimizer->GetCurrentParameters();
    optimizer->NextIteration( trace );
  }

  double modelMeanX;
  double modelMeanY;
  model->GetMeans( modelMeanX, modelMeanY );

  if ( fabs( modelMeanX - currentParameters[0] ) > 1.0e-3 ||
       fabs( modelMeanY - currentParameters[1] ) > 1.0e-3 ) {
    std::cerr << "RegularStepGradientDescentOptimizer failed to converge "
              << "on the expected solution." << std::endl;
    std::cerr << "Expected currentParameters to be (" << modelMeanX << ", "
              << modelMeanY << "), got (" << currentParameters[0] << ", "
              << currentParameters[1] << ") instead." << std::endl;
    return EXIT_FAILURE;
  }

  delete trace;

  return EXIT_SUCCESS;
}
