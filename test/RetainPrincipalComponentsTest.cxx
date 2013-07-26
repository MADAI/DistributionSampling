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

  std::cout << "Eigenvalues:\n" << emulator.m_PCAEigenvalues << "\n";
  std::cout << "Eigenvectors:\n" << emulator.m_PCAEigenvectors << "\n";

  // Retain all components
  emulator.RetainPrincipalComponents( 1.0 );
  
  if ( emulator.m_RetainedPCAEigenvalues.rows() !=
       emulator.m_PCAEigenvalues.rows() ) {
    std::cerr << "Unexpected number of eigenvalue rows with fractional resolving power 1.0\n";
    return EXIT_FAILURE;
  }

  if ( emulator.m_RetainedPCAEigenvalues.cols() !=
       emulator.m_PCAEigenvalues.cols() ) {
    std::cerr << "Unexpected number of eigenvalue columns with fractional resolving power 1.0\n";
    return EXIT_FAILURE;
  }

  if ( (emulator.m_RetainedPCAEigenvalues - emulator.m_PCAEigenvalues).maxCoeff() > 1e-4) {
    std::cerr << "Mismatch in eigenvalues for fractional resolving power 1.0\n";
    return EXIT_FAILURE;
  }

  if ( emulator.m_RetainedPCAEigenvectors.rows() !=
       emulator.m_PCAEigenvectors.rows() ) {
    std::cerr << "Unexpected number of eigenvector rows with fractional resolving power 1.0\n";
    return EXIT_FAILURE;
  }

  if ( emulator.m_RetainedPCAEigenvectors.cols() !=
       emulator.m_PCAEigenvectors.cols() ) {
    std::cerr << "Unexpected number of eigenvector columns with fractional resolving power 1.0\n";
    return EXIT_FAILURE;
  }

  if ( (emulator.m_RetainedPCAEigenvectors - emulator.m_PCAEigenvectors).maxCoeff() > 1e-4) {
    std::cerr << "Mismatch in eigenvectors for fractional resolving power 1.0\n";
    return EXIT_FAILURE;
  }

  // Retain some of the components
  emulator.RetainPrincipalComponents( 0.5 );

  // There should be only two retained eigenvalues and eigenvector columns
  if ( emulator.m_RetainedPCAEigenvalues.rows() != 2 ) {
    std::cerr << "Wrong number of retained eigenvalues for fractional "
              << "resolving power 0.5\n";
    return EXIT_FAILURE;
  }

  if ( emulator.m_RetainedPCAEigenvectors.rows() != 3 ||
       emulator.m_RetainedPCAEigenvectors.cols() != 2 ) {
    std::cerr << "Wrong retained eigenvector matrix size for fractional "
              << "resolving power 0.5\n";
    return EXIT_FAILURE;
  }

  if ( (emulator.m_RetainedPCAEigenvectors -
        emulator.m_PCAEigenvectors.rightCols( 2 ) ).maxCoeff() > 1e-4 ) {
    std::cerr << "Wrong retained eigenvector matrix for fractional "
              << "resolving power 0.5\n";
    return EXIT_FAILURE;
  }

  std::cout << "Retained eigenvalues: \n"
            << emulator.m_RetainedPCAEigenvalues << "\n";
  std::cout << "Retained eigenvectors: \n"
            << emulator.m_RetainedPCAEigenvectors << "\n";

  return EXIT_SUCCESS;
}
