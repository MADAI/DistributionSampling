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
#include <iostream>
#include <cmath>

#include "Gaussian2DModel.h"
#include "LangevinSampler.h"
#include "Trace.h"


int main( int, char *[] )
{
  madai::Gaussian2DModel model;

  madai::LangevinSampler sampler;
  sampler.SetModel( &model );
  sampler.SetTimeStep( 0.1 );
  sampler.SetKickStrength( 0.1 );
  sampler.SetMeanTimeBetweenKicks( 50 );
  sampler.SetDragCoefficient( 0.1 );
  sampler.SetMassScale( 5 );

  sampler.SetParameterValue( "X", 13 );
  sampler.SetParameterValue( "Y", -2 );
  sampler.SetVelocity( "X", 0.0 );
  sampler.SetVelocity( "Y", 0.0 );
  madai::Sample BestSample;
  madai::Sample NewSample;

  BestSample.m_LogLikelihood = -1000;
  for ( unsigned int i = 0; i < 100000; ++i ) {
    NewSample = sampler.NextSample();
    if ( NewSample.m_LogLikelihood > BestSample.m_LogLikelihood ) {
      BestSample = NewSample;
    }
  }

  double Means[2];
  model.GetMeans( Means[0], Means[1] );

  double diff1 = Means[0] - BestSample.m_ParameterValues[0];
  if ( diff1 < 0 ) diff1 *= -1;
  double diff2 = Means[1] - BestSample.m_ParameterValues[1];
  if ( diff2 < 0 ) diff2 *= -1;

  if ( diff1 > 1.0e-1 && diff2 > 1.0e-1 ) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
