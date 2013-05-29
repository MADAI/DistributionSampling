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

#ifndef madai_LatinHypercubeGenerator_h_included
#define madai_LatinHypercubeGenerator_h_included

#include <vector>

#include "Parameter.h"
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

  /** Set the number of standard deviations about the mean used to
   * determine the bounds of the latin hypercube sampling for
   * parameters with Gaussian priors. Defaults to 3.0.
   *
   * This is only used when DivideSpaceByPercentile is enabled. */
  void SetStandardDeviations( double standardDeviations );
  double GetStandardDeviations() const;

  /** If enabled, this option partitions the parameter space according to
   * the percentile of the prior distribution in each dimension. If
   * disabled, each dimension is divided up evenly. This is off by
   * default. */
  void SetPartitionSpaceByPercentile( bool value );
  bool GetPartitionSpaceByPercentile() const;


  /** Generates a list of parameters distributed in a high-dimensional
   * parameter space according to a Latin hypercube sampling pattern.
   * http://en.wikipedia.org/wiki/Latin_hypercube_sampling
   *
   * Note that the output Samples have only parameter values, no
   * output values nor log likelihoods. */
  std::vector< Sample > Generate( int numberOfTrainingPoints,
                                  const std::vector< Parameter > parameters );

  /** Generates a list of parameters distributed in a high-dimensional
   * parameter space according to a Latin hypercube sampling pattern.
   * http://en.wikipedia.org/wiki/Latin_hypercube_sampling
   *
   * Note that the output Samples have only parameter values, no
   * output values nor log likelihoods.
   *
   * Repeat the process @param numberOfTries times, and return the
   * Latin hypercube sampling with the best spacing.
   */
  std::vector< Sample > GenerateMaxiMin(
      int numberOfTrainingPoints,
      const std::vector< Parameter > parameters,
      int numberOfTries);

protected:
  Random * m_Random;

  double m_StandardDeviations;

  bool m_PartitionSpaceByPercentile;

  void PartitionDimension( int numberOfTrainingPoints,
                           const Parameter & parameter,
                           std::vector< double > & subdivisions );

};

} // end namespace madai

#endif // madai_LatinHypercubeGenerator_h_included
