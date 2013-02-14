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

#include "SimpleMetropolisHastings.h"

#include <cstdlib>


namespace madai {


SimpleMetropolisHastings
::SimpleMetropolisHastings( const Model * model ) :
  Optimizer( model ),
  m_StepSize(1.0e-2),
  m_ActiveParameters( model->GetNumberOfParameters(), true ),
  m_NumberOfParameters( model->GetNumberOfParameters() ),
  m_NumberOfOutputs( model->GetNumberOfScalarOutputs() )
{
}


SimpleMetropolisHastings
::~SimpleMetropolisHastings()
{
}


void SimpleMetropolisHastings
::SetStepSize( double stepSize )
{
  this->m_StepSize = stepSize;
}


static double uniform_rand()
 {
  // fixme better algorithm, seed.
  const static double R = 1.0 / static_cast<double>(RAND_MAX);
  return R * static_cast<double>(rand());
}


void SimpleMetropolisHastings
::NextIteration( Trace *trace )
{
  // xc is x_candidate
  std::vector< double > xc( this->m_NumberOfParameters, 0.0 );
  std::vector< double > yc( this->m_NumberOfOutputs, 0.0 );
  unsigned int trace_length = trace->length();
  if ( trace_length < 1 ) {
    for ( unsigned int i = 0; i < this->m_NumberOfParameters; i++ ) {
      double range[2];
      this->m_Model->GetRange( i, range );
      xc[i] = 0.5 * (range[0] + range[1]);
      //FIXME should be random.
    }
    this->m_Model->GetScalarOutputs( xc, yc );
    trace->add( xc, yc );
    trace_length ++;
  }

  std::vector< double > * x = &((*trace)[trace_length-1].m_ParameterValues);
  std::vector< double > * y = &((*trace)[trace_length-1].m_OutputValues);

  unsigned int output_index = this->GetOutputScalarToOptimizeIndex();

  double f_of_x = (*y)[output_index];

  for ( unsigned int giveup = 1048576; giveup != 0; --giveup ) {
    for (unsigned int i = 0; i < this->m_NumberOfParameters; i++) {
      if (this->m_ActiveParameters[i]) {
        xc[i] = (*x)[i] + (uniform_rand() - 0.5) * 2 * this->m_StepSize;
      } else {
        xc[i] = (*x)[i];
      }
    }

    this->m_Model->GetScalarOutputs(xc, yc);
    double f_of_xc = yc[output_index];
    double a = f_of_xc / f_of_x;
    if ((a > 1.0) || (a > uniform_rand())) {
      trace->add(xc, yc);
      return;
    }
  }
}

} // end namespace madai
