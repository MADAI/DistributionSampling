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

#include <cassert>

#include "SamplerCSVWriter.h"
#include "Trace.h"
#include "Sampler.h"
#include "Model.h"

namespace madai {


int SamplerCSVWriter
::GenerateSamplesAndSaveToFile(
    Sampler * sampler,
    Model * model,
    std::ostream & outFile,
    int NumberOfSamples,
    int NumberOfBurnInSamples,
    bool UseEmulatorCovariance,
    std::ostream * progress)
{
  assert(model != NULL);
  assert(sampler != NULL);
  model->SetUseModelCovarianceToCalulateLogLikelihood(UseEmulatorCovariance);
  sampler->SetModel( model );
  int step = NumberOfBurnInSamples / 100, percent = 0;
  if ( step < 1 ) {
    step = 1; // avoid div-by-zero error
  }
  for ( int count = 0; count < NumberOfBurnInSamples; count++ ) {
    if (progress != NULL) {
      if ( count % step == 0 ) {
        (*progress) << '\r' << "Burn in percent done: " << percent++ << "%";
        progress->flush();
      }
    }
    sampler->NextSample(); // Discard samples in the burn-in phase
  }
  step = NumberOfSamples / 100, percent = 0;
  if ( step < 1 ) {
    step = 1; // avoid div-by-zero error
  }
  madai::Trace trace;
  for (int count = 0; count < NumberOfSamples; count ++) {
    if (progress != NULL) {
      if (count % step == 0)
        (*progress) <<  '\r' << "SAMPLER percent done: " << percent++ << "%";
      progress->flush();
    }
    trace.Add( sampler->NextSample() );
  }
  if (progress != NULL) {
    (*progress) << "\r                          \r";
    progress->flush();
  }
  trace.WriteCSVOutput(
      outFile, model->GetParameters(), model->GetScalarOutputNames() );
  return EXIT_SUCCESS;
}

} // end namespace madai
