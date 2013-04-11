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
 
 /**
PCADecompose
   Decompose the model data from a directory structure.
*/

const char useage [] =
  "Useage:\n"
  "    PCADecompose RootDirectory [fractionResolvingPower]\n"
  "\n"
  "RootDirectory is the directory containing the folders model_output/\n"
  "experimental_reults/ and statisical_analysis/ are contained\n"
  "\n"
  "[fractionResolvingPower is the faction of the power of the emulator\n"
  "at resolving the data into different components. Default .95\n";

#include <iostream>
#include <fstream>

#include "GaussianProcessEmulator.h"

int main( int argc, char ** argv ) {
  std::string TopDirectory;
  double fractionResolvingPower = 0.95;
  if ( argc > 1 ) {
    TopDirectory = std::string(argv[1]);
    if ( argc > 2 ) {
      fractionResolvingPower = atof( argv[2] );
    }
  }
  std::string outputFileName = TopDirectory
      +"/statistical_analysis/PCADecomposition.dat";
  
  madai::GaussianProcessEmulator gpme;
  if ( TopDirectory == "-" ) {
    gpme.LoadTrainingData(std::cin);
  } else {
    if ( !gpme.LoadTrainingData(TopDirectory) ) {
      std::cerr << "Error Loading Training Data.\n";
      return EXIT_FAILURE;
    }
  }
  // get the PCA decomposition
  if ( !gpme.PrincipalComponentDecompose( fractionResolvingPower ) )
    return EXIT_FAILURE;
  std::ofstream os( outputFileName.c_str() );
  gpme.WritePCA(os);
  return EXIT_SUCCESS;
}










