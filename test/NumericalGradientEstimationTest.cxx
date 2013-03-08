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
#include "VectorUtilities.h"


int main( int argc, char *argv[] )
{
  madai::Gaussian2DModel *model = new madai::Gaussian2DModel();

  // Get the known gradient value
  std::vector< double > parameters;
  parameters.push_back( 22.2 );
  parameters.push_back( -13.0 );

  std::vector< double > scalars;
  std::vector< double > actualGradient;
  std::vector< bool >   activeParameters;

  // Test all parameters as active
  for ( unsigned int i = 0; i < model->GetNumberOfParameters(); ++i ) {
    activeParameters.push_back( true );
  }

  unsigned int outputIndex = 0;
  madai::Model::ErrorType error =
    model->GetScalarAndGradientOutputs( parameters, activeParameters,
                                        scalars, outputIndex, actualGradient );

  std::vector< double > estimatedGradient;
  error =
    model->Model::GetScalarAndGradientOutputs( parameters, activeParameters,
                                               scalars, outputIndex,
                                               estimatedGradient );

  unsigned int activeParameterIndex = 0;
  for ( unsigned int i = 0; i < model->GetNumberOfParameters(); ++i ) {

    if ( activeParameters[i] ) {
      double actual   = actualGradient[ activeParameterIndex ];
      double estimate = estimatedGradient[ activeParameterIndex ];
      if ( std::abs( actual - estimate ) > 1.0e-5 ) {
        std::cerr << "Actual gradient and estimated gradient differ at "
                  << "active parameter " << i << std::endl;
        std::cout << "actualGradient: " << actualGradient << std::endl;
        std::cout << "estimatedGradient: " << estimatedGradient << std::endl;
        return EXIT_FAILURE;
      }
    }
  }

  // We'll save the actual gradient to compare to gradient returned
  // when only one parameter is active
  std::vector< double > partialGradient;

  activeParameters[1] = false;
  error =
    model->Model::GetScalarAndGradientOutputs( parameters, activeParameters,
                                               scalars, outputIndex,
                                               partialGradient );
  double partial    = partialGradient[0];
  double estimated  = estimatedGradient[0];
  if ( fabs( estimated - partial ) > 1.0e-5 ) {
    std::cerr << "Unexpected gradient component for active parameter 0" << std::endl;
    std::cerr << "Expected " << estimated << ", got " << partial << std::endl;
    return EXIT_FAILURE;
  }

  activeParameters[0] = false;
  activeParameters[1] = true;
  error =
    model->Model::GetScalarAndGradientOutputs( parameters, activeParameters,
                                               scalars, outputIndex,
                                               partialGradient );

  partial   = partialGradient[0];
  estimated = estimatedGradient[1];
  if ( fabs( estimated - partial ) > 1.0e-5 ) {
    std::cerr << "Unexpected gradient component for active parameter 1" << std::endl;
    std::cerr << "Expected " << estimated << ", got " << partial << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "NumericalGradientEstimationTest passed" << std::endl;

  return EXIT_SUCCESS;
}
