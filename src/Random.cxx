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
extern "C" {
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
}
#include <time.h>

#include "Random.h"

namespace madai {
/**
 * Constructor.  Uses time() for seed.
 */
Random::Random()
{
  this->m_rng = gsl_rng_alloc (gsl_rng_mt19937);
  this->Reseed();
}

/**
 * Constructor.
 */
Random::Random(unsigned long int seed)
{
  this->m_rng = gsl_rng_alloc (gsl_rng_taus);
  this->Reseed(seed);
}

/**
 * Destructor.
 */
Random::~Random()
{
  gsl_rng_free(this->m_rng);
}

/**
 * reseed
 */
void Random::Reseed(unsigned long int seed)
{
  gsl_rng_set(this->m_rng, seed);
}

/**
 * reseed with time()
 */
void Random::Reseed()
{
  this->Reseed(time(NULL));
}

/**
 * Returns an integer < N and >= 0
 */
double Random::Integer(unsigned long int N)
{
  return gsl_rng_uniform_int (this->m_rng, N);
}

/**
 * min=0.0, max=1.0
 */
double Random::Uniform()
{
  return gsl_rng_uniform(this->m_rng);
}

/**
 *
 */
double Random::Uniform(double min, double max)
{
  return min + (gsl_rng_uniform(this->m_rng) / (max - min));
}

/**
 * mean=0.0, var=1.0
 */
double Random::Gaussian()
{
  return gsl_ran_ugaussian(this->m_rng);
}

/**
 *
 */
double Random::Gaussian(double mean, double standardDeviation)
{
  return mean + gsl_ran_gaussian (this->m_rng, standardDeviation);
}

};
