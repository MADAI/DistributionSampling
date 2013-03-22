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

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "LatinHypercubeGenerator.h"
#include "Parameter.h"

void printElement( double d ) {
  std::cout << d << " ";
}


int main( int argc, char* argv[] )
{
  int numberOfTrainingPoints = 4;

  std::vector< madai::Parameter > parameters;
  parameters.push_back( madai::Parameter( "param_0", -1.0,  1.0 ) );
  parameters.push_back( madai::Parameter( "param_1",  2.1,  3.2 ) );
  parameters.push_back( madai::Parameter( "param_2", -4.7, -2.2 ) );

  madai::LatinHypercubeGenerator generator;
  std::vector< madai::Sample > samples = 
    generator.Generate( numberOfTrainingPoints, parameters );

  std::cout << "Number of samples: " << samples.size() << std::endl;
  for ( size_t i = 0; i < samples.size(); ++i ) {
    madai::Sample sample = samples[i];
    for ( size_t j = 0; j < sample.m_ParameterValues.size(); ++j ) {
      std::cout << sample.m_ParameterValues[j] << ", ";
    }
    std::cout << std::endl;
  }

  // Test whether a valid latin hypercube was generated.
  std::vector< std::vector< double > > parametersForDimension;
  for ( size_t i = 0; i < parameters.size(); ++i ) {
    parametersForDimension.push_back( std::vector< double >( samples.size(), 0.0 ) );
  }

  for ( size_t i = 0; i < samples.size(); ++i ) {
    madai::Sample sample = samples[i];
    for ( size_t j = 0; j < parametersForDimension.size(); ++j ) {
      parametersForDimension[j][i] = sample.m_ParameterValues[j];
    }
  }

  // Sort the parameter vectors
  for ( size_t j = 0; j < parametersForDimension.size(); ++j ) {
    std::sort( parametersForDimension[j].begin(),
               parametersForDimension[j].end() );

    std::cout << "Sorted elements for parameter " << j << ":\n";
    std::for_each( parametersForDimension[j].begin(),
                   parametersForDimension[j].end(),
                   printElement );
    std::cout << "\n";

    // Now check that the spacing between the parameters in each
    // dimension is equal to within numerical precision.
    double expectedDiff = parametersForDimension[j][1] - parametersForDimension[j][0];
    for ( size_t i = 1; i < parametersForDimension[j].size()-1; ++i ) {
      double i1 = parametersForDimension[j][i];
      double i2 = parametersForDimension[j][i+1];
      if ( fabs( i2 - i1 - expectedDiff ) > 1e-4 ) {
        std::cerr << "Incorrect spacing between parameter samples in dimension "
                  << i << ". Should have been " << expectedDiff << ", was "
                  << (i2 - i1 ) << "\n";
        return EXIT_FAILURE;
      }
    }    
  }

  return EXIT_SUCCESS;
}

