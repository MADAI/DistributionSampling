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
#include <cmath>

#include "Gaussian2DModel.h"
#include "LangevinSampler.h"
#include "Trace.h"


int main( int argc, char *arg[] )
{
  madai::Gaussian2DModel model;
  
  madai::LangevinSampler sampler;
  sampler.SetModel( &model );
  sampler.SetTimeStep( 0.1 );
  sampler.SetKickStrength( 10 );
  sampler.SetMeanTimeBetweenKicks( 5 );
  sampler.SetDragCoefficient( 0.1 );
  sampler.SetMassScale( 5 );
  
  sampler.SetParameterValue( "X", 13 );
  sampler.SetParameterValue( "Y", -4 );
  sampler.SetVelocity( "X", 0.0 );
  sampler.SetVelocity( "Y", 0.0 );
  
  madai::Trace trace;
  
  for ( unsigned int i = 0; i < 100; ++i ) {
    trace.Add( sampler.NextSample() );
  }
  
  return EXIT_SUCCESS;
}