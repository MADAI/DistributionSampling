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

#include <cstdlib>

#include "GaussianProcessEmulator.h"


int main( int, char *[] ) {
  
  madai::GaussianProcessEmulator emulator;
  emulator.m_NumberOutputs = 3; // X, Y, and Z coordinates
  emulator.m_NumberTrainingPoints = 30;

  emulator.m_TrainingOutputVarianceMeans = Eigen::MatrixXd( 3, 1 );
  emulator.m_ObservedVariances = Eigen::MatrixXd( 3, 1 );
  for ( int i = 0; i < 3; ++i ) {
    emulator.m_TrainingOutputVarianceMeans( i, 0 ) = 0.0;
    emulator.m_ObservedVariances( i, 0 ) = 1.0;
  }

  // Training data
  int numSamplesInDimension = 10;
  int spacing[3] = {1.0, 2.0, 1.0};
  emulator.m_TrainingOutputValues =
    Eigen::MatrixXd( 3*numSamplesInDimension, 3 );

  for ( int i = 0; i < numSamplesInDimension; ++i ) {
    // Point along x dimension
    emulator.m_TrainingOutputValues( i, 0 ) =
      (i * spacing[0]) - (0.5 * (numSamplesInDimension-1) * spacing[0]);
    emulator.m_TrainingOutputValues( i, 1 ) = emulator.m_TrainingOutputValues( i, 0 );
    emulator.m_TrainingOutputValues( i, 2 ) = 0.0;

    // Point along y dimension
    emulator.m_TrainingOutputValues( i+numSamplesInDimension, 0 ) = 0.0;
    emulator.m_TrainingOutputValues( i+numSamplesInDimension, 1 ) =
      (i * spacing[1]) - (0.5 * (numSamplesInDimension-1) * spacing[1]);
    emulator.m_TrainingOutputValues( i+numSamplesInDimension, 2 ) =
      emulator.m_TrainingOutputValues( i+numSamplesInDimension, 1 );

    // Point along z dimension
    emulator.m_TrainingOutputValues( i+(2*numSamplesInDimension), 0 ) = 0.0;
    emulator.m_TrainingOutputValues( i+(2*numSamplesInDimension), 1 ) = 0.0;
    emulator.m_TrainingOutputValues( i+(2*numSamplesInDimension), 2 ) =
      (i * spacing[2]) - (0.5 * (numSamplesInDimension-1) * spacing[2]);
  }

  emulator.PrincipalComponentDecompose();

  // Expected eigenvalues (verified in MATLAB)
  Eigen::MatrixXd expectedEigenvalues( 3, 1 );
  expectedEigenvalues( 0, 0 ) = 0.72517;
  expectedEigenvalues( 1, 0 ) = 4.60297;
  expectedEigenvalues( 2, 0 ) = 24.9219;

  if ( (expectedEigenvalues - emulator.m_PCAEigenvalues).maxCoeff() > 1e-4 ) {
    std::cerr << "Unexpected eigenvalues\n";
    std::cerr << "Got: \n" << emulator.m_PCAEigenvalues << "\n";
    std::cerr << "Expected: \n" << expectedEigenvalues << "\n";
    return EXIT_FAILURE;
  }

  // Expected eigenvectors (verified in MATLAB)
  Eigen::MatrixXd expectedEigenvectors( 3, 3 );
  expectedEigenvectors( 0, 0 ) = 0.720036;
  expectedEigenvectors( 0, 1 ) = 0.68833;
  expectedEigenvectors( 0, 2 ) = 0.0880372;

  expectedEigenvectors( 1, 0 ) = -0.530164;
  expectedEigenvectors( 1, 1 ) = 0.463801;
  expectedEigenvectors( 1, 2 ) = 0.7098;

  expectedEigenvectors( 2, 0 ) = 0.447745;
  expectedEigenvectors( 2, 1 ) = -0.557755;
  expectedEigenvectors( 2, 2 ) = 0.69888;

  if ( (expectedEigenvectors - emulator.m_PCAEigenvectors).maxCoeff() > 1e-4 ) {
    std::cerr << "Unexpected eigenvectors\n";
    std::cerr << "Got: \n" << emulator.m_PCAEigenvectors << "\n";
    std::cerr << "Expected: \n" << expectedEigenvectors << "\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
