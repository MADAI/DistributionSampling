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
#include <vector>

#include "LatinHypercubeGenerator.h"
#include "Parameter.h"


int main( int argc, char* argv[] )
{
  int numberOfTrainingPoints = 4;

  std::vector< madai::Parameter > parameters;
  parameters.push_back( madai::Parameter( "param_0", 0.0, 1.0 ) );
  parameters.push_back( madai::Parameter( "param_1", 0.0, 1.0 ) );
  parameters.push_back( madai::Parameter( "param_2", 0.0, 1.0 ) );

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

  // \todo - test whether a valid latin hypercube was generated

  return EXIT_SUCCESS;
}

