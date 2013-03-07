/*=========================================================================
 *
 *  Copyright 2013 The University of North Carolina at Chapel Hill
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

#ifndef MADAI_RANDOM_H
#define MADAI_RANDOM_H

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>

/**
 * A random number generator.  contains a seed.
 */
namespace madai {

class Random {
public:
  /**
   * Constructor.  Uses time() for seed.
   */
  Random();
  /**
   * Constructor.
   */
  Random(unsigned long int seed);

  /**
   * Destructor.
   */
  virtual ~Random();
  /**
   * reseed
   */
  virtual void Reseed(unsigned long int seed);
  /**
   * reseed with time()
   */
  virtual void Reseed();
  /**
   * Returns an integer < N and >= 0
   */
  virtual int Integer(unsigned long int N);

  /**
   * min=0.0, max=1.0
   */
  virtual double Uniform();
  /**
   *
   */
  virtual double Uniform(double min, double max);
  /**
   * mean=0.0, var=1.0
   */
  virtual double Gaussian();
  /**
   *
   */
  virtual double Gaussian(double mean, double standardDeviation);

private:
  /** explicitly disallowed */
  Random& operator=(madai::Random &);
  /** explicitly disallowed */
  Random(madai::Random const &);

  typedef boost::mt19937                                          BaseGeneratorType;
  typedef boost::uniform_int<>                                    UniformIntDistributionType;
  typedef boost::uniform_real<>                                   UniformRealDistributionType;
  typedef boost::normal_distribution<>                            NormalDistributionType;
  typedef boost::variate_generator< BaseGeneratorType&,
                                    UniformRealDistributionType > UniformRealGeneratorType;

  /** Uniform random integer generator. */
  BaseGeneratorType           m_BaseGenerator;
  UniformIntDistributionType  m_UniformIntDistribution;
  UniformRealDistributionType m_UniformRealDistribution;
  UniformRealGeneratorType    m_UniformRealGenerator;
  NormalDistributionType      m_NormalDistribution;

};

}

#endif /* MADAI_RANDOM_H */
