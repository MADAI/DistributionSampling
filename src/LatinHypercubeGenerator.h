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

#ifndef madai_LatinHypercubeGenerator_h_included
#define madai_LatinHypercubeGenerator_h_included

#include <vector>

#include "Sample.h"


namespace madai {

// Forward declaration
class Random;


/** \class LatinHypercubeGenerator
 *
 * This class is used to generate a latin hypercube sampling of a
 * parameter space. */
class LatinHypercubeGenerator {
public:
  /** Constructor */
  LatinHypercubeGenerator();

  /** Destructor */
  virtual ~LatinHypercubeGenerator();

  /** Generates a list of parameters distributed in a high-dimensional
   * parameter space according to a Latin hypercube sampling pattern:
   * http://en.wikipedia.org/wiki/Latin_hypercube_sampling
   *
   * Note that the output Samples have only parameter values, no
   * output values nor log likelihoods. */
  std::vector< Sample > Generate( int numberOfParameters,
                                  int numberOfTrainingPoints,
                                  const std::vector< double > & parameterMinima,
                                  const std::vector< double > & parameterMaxima );

protected:
  Random * m_Random;

};

} // end namespace madai

#endif // madai_LatinHypercubeGenerator_h_included
