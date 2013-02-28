/*=========================================================================
 *
 *  Copyright The University of North Carolina at Chapel Hill
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

#include "SimpleMetropolisHastingsSampler.h"

#include <cassert>
#include <cmath> // std::sin, cos, log, sqrt
#include <algorithm> // std::count

namespace madai {


SimpleMetropolisHastingsSampler
::SimpleMetropolisHastingsSampler( const Model * model ) :
  Sampler( model ),
  m_StepSize( 1.0e-2 ),
  m_NumberOfParameters( model->GetNumberOfParameters() ),
  m_NumberOfOutputs( model->GetNumberOfScalarOutputs() ),
  m_LastStepParameters( model->GetNumberOfParameters() ),
  m_LastStepOutputs( model->GetNumberOfScalarOutputs() ),
  m_LastStepLogLikelihood (-1e30)
{
  assert(model != NULL);
  for ( unsigned int i = 0; i < m_NumberOfParameters; i++ ) {
    double range[2];
    m_Model->GetRange( i, range );
    m_LastStepParameters[i] = 0.5 * (range[0] + range[1]);
    //FIXME should we have a random starting point?
  }
  Model * m = const_cast< Model * >(m_Model);
  m->GetScalarOutputsAndLogLikelihood(
    m_LastStepParameters,
    m_LastStepOutputs,
    m_LastStepLogLikelihood); // initial starting point.
}


SimpleMetropolisHastingsSampler
::~SimpleMetropolisHastingsSampler()
{
}


void SimpleMetropolisHastingsSampler
::SetStepSize( double stepSize )
{
  m_StepSize = stepSize;
}


static inline double uniform_rand()
 {
  // fixme better algorithm, seed.
  const static double R = 1.0 / static_cast<double>(RAND_MAX);
  return R * static_cast<double>(rand());
}

static inline void random_multinormal(double *d, size_t n,
                                      double * sqmodulus = NULL)
{
  //Box-Muller
  static const double TWO_PI = 6.28318530718;
  double sum_of_squares = 0.0;
  for (size_t i = 0; i < n ; /* intentionally left blank */ ) {
    double r = std::sqrt(-2.0 * std::log(uniform_rand()));
    double theta = TWO_PI * uniform_rand();
    double x = r * std::cos(theta);
    sum_of_squares += (x * x);
    d[i++] = x;
    if (i < n) {
      double y = r * std::sin(theta);
      sum_of_squares += (y * y);
      d[i++] = y;
    }
  }
  if (sqmodulus != NULL)
    *sqmodulus = sum_of_squares;
}
static inline void random_inside_hypersphere(
    double scale, double *d, size_t n)
{
  double sqmodulus = 0.0;
  random_multinormal(d, n, &sqmodulus);
  scale *= (uniform_rand() / std::sqrt(sqmodulus));
  for (size_t i = 0; i < n ; ++i )
    d[i] *= scale;
}


void
SimpleMetropolisHastingsSampler
::NextSample( Trace *trace )
{
  // xc is x_candidate
  Model * m = const_cast< Model * >(m_Model);

  std::vector< double > xc( m_NumberOfParameters, 0.0 );
  std::vector< double > yc( m_NumberOfOutputs, 0.0 );

  unsigned int output_index = this->GetOutputScalarToOptimizeIndex();
  unsigned int numberOfActiveParameters = this->GetNumberOfActiveParameters();
  std::vector< double > randomStep( numberOfActiveParameters, 0.0 );

  assert( std::count(this->m_ActiveParameterIndices.begin(),
    this->m_ActiveParameterIndices.end(), true) == numberOfActiveParameters);

  for ( unsigned int giveup = 1048576; giveup != 0; --giveup ) {
    random_inside_hypersphere(
      m_StepSize, &(randomStep[0]), numberOfActiveParameters);
    unsigned int k = 0;
    for(unsigned int i = 0; i < m_NumberOfParameters; i++) {
      if (this->m_ActiveParameterIndices[i]) {
        xc[i] = m_LastStepParameters[i] + randomStep[k];
        k++;
      } else {
        xc[i] = m_LastStepParameters[i];
				// Not sure how to respect this->m_CurrentParameters[i] ???
				// for (i < m_NumberOfParameters)
				//   if (!this->m_ActiveParameterIndices[i] &&
				//        (this->m_CurrentParameters[i] != m_LastStepParameters[i])) {
				//     do something;
				//   }
      }
    }

    if (m_OptimizeOnLikelihood) {
      double ll;
      m->GetScalarOutputsAndLogLikelihood(xc,yc,ll);
      double delta_logLikelihood = ll - m_LastStepLogLikelihood;
      if ((delta_logLikelihood > 0) ||
          (exp(delta_logLikelihood) > uniform_rand()))
        {
          m_LastStepLogLikelihood = ll;
          m_LastStepParameters = xc;
          m_LastStepOutputs = yc;
          trace->add(xc, yc, ll);
          return;
          //FIXME do somthing
        }
    } else {
      double ll;
      m->GetScalarOutputsAndLogLikelihood(xc,yc,ll);
      //m_Model->GetScalarOutputs(xc,yx);
      double f_of_x = m_LastStepOutputs[output_index];
      double f_of_xc = yc[output_index];
      if ((f_of_xc > f_of_x) || (f_of_xc > (f_of_x * uniform_rand())))
        {
          m_LastStepLogLikelihood = ll;
          m_LastStepParameters = xc;
          m_LastStepOutputs = yc;
          trace->add(xc, yc, ll);
          return;
          //FIXME do somthing ???
        }
    }
  }
}

} // end namespace madai
