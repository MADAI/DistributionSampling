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
#include <cstdlib>
#include <iomanip>

#include "SamplerCSVWriter.h"
#include "Sample.h"
#include "Sampler.h"
#include "Model.h"

namespace madai {


int SamplerCSVWriter
::GenerateSamplesAndSaveToFile(
    Sampler & sampler,
    Model & model,
    std::ostream & outFile,
    int NumberOfSamples,
    int NumberOfBurnInSamples,
    bool UseEmulatorCovariance,
    std::ostream * progress)
{
  model.SetUseModelCovarianceToCalulateLogLikelihood(UseEmulatorCovariance);
  sampler.SetModel( &model );

  enum phase { burnIn = 0, traceGeneration = 1 };
  int currentNumberOfSamples[2] = { NumberOfBurnInSamples, NumberOfSamples };
  const char * currentName[2] = {"Burn in", "Sampler"};
  double bestLogLikelihood = 0;
  Sample oldSample = sampler.NextSample();
  for ( int currentPhase = burnIn; currentPhase <= traceGeneration; currentPhase++ ) {
    int step = currentNumberOfSamples[currentPhase] / 100, percent = 1;
    if ( step < 1 ) {
      step = 1; // avoid div-by-zero error
    }
    int successfulSteps = 0, failedSteps = 0;
    for ( int count = 0; count < currentNumberOfSamples[currentPhase]; count++ ) {
      Sample sample = sampler.NextSample();
      if ( sample == oldSample ) {
        failedSteps++;
      }
      else {
        successfulSteps++;
      }
      oldSample = sample;

      WriteSample( outFile, sample );

      if ( sample.m_LogLikelihood > bestLogLikelihood || ( currentPhase == burnIn && count == 0 ) ) {
        bestLogLikelihood = sample.m_LogLikelihood;
      }

      if ( progress != NULL ) {
        if ( ( count + 1 ) % step == 0 ) {
          (*progress) <<  '\r' << currentName[currentPhase] << " percent done: " << std::setfill('0') << std::setw(2) << percent++ << "%";
          (*progress) << "  Success rate: " << std::setfill('0') << std::setw(2) << 100*successfulSteps / (successfulSteps + failedSteps) << "%";
          (*progress) << "  Best log likelihood: " << bestLogLikelihood;
        }
        progress->flush();
      }
    }
    if ( currentPhase == burnIn ) {
      WriteHeader( outFile, model.GetParameters(), model.GetScalarOutputNames() );
    }
    if ( progress != NULL ) {
      // Leave the success rate percentage visible
      (*progress) << "\n";
      progress->flush();
    }
  }

  return EXIT_SUCCESS;
}


void
SamplerCSVWriter
::WriteHeader( std::ostream & o,
               const std::vector< Parameter > & params,
               const std::vector< std::string > & outputs)
{
  if ( !params.empty() ) {
    std::vector< Parameter >::const_iterator itr = params.begin();
    o << '"' << itr->m_Name << '"';
    for ( itr++; itr < params.end(); itr++ ) {
      o << ',' << '"' << itr->m_Name << '"';
    }
    if ( !outputs.empty() ) {
      o << ',';
    }
  }
  if ( !outputs.empty() ) {
    std::vector<std::string>::const_iterator itr = outputs.begin();
    //std::cout << "Output name: " << *itr << std::endl;
    o << '"' << *itr << '"';
    for ( itr++; itr < outputs.end(); itr++ ) {
      o << ',' << '"' << *itr << '"';
    }
  }
  o << ",\"LogLikelihood\"\n";
}


/** Utility for writing a vector to an output stream
 *
 * The vector elements are delimited by the given delimiter */
template <class T>
void write_vector( std::ostream& o, std::vector< T > const & v, char delim ) {
  if ( !v.empty() ) {
    typename std::vector< T >::const_iterator itr = v.begin();
    o << *(itr++);
    while ( itr < v.end() ) {
      o << delim << *(itr++);
    }
  }
}


void
SamplerCSVWriter
::WriteSample( std::ostream & out, const Sample & sample )
{
  write_vector( out, sample.m_ParameterValues, ',' );
  out << ',';

  if ( sample.m_OutputValues.size() > 0 ) {
    write_vector( out, sample.m_OutputValues, ',' );
    out << ',';
  }
  out << sample.m_LogLikelihood;
  if ( sample.m_Comments.size() > 0 ) {
    out << ",\"";
    write_vector( out, sample.m_Comments, ';' );
    out << '"';
  }
  out << '\n';

  out.flush();
}


} // end namespace madai
