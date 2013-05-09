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
    sampler.NextSample(); // Discard samples in the burn-in phase
  }
  step = NumberOfSamples / 100, percent = 0;
  if ( step < 1 ) {
    step = 1; // avoid div-by-zero error
  }

  WriteHeader( outFile, model.GetParameters(), model.GetScalarOutputNames() );

  for (int count = 0; count < NumberOfSamples; count ++) {
    if (progress != NULL) {
      if (count % step == 0)
        (*progress) <<  '\r' << "SAMPLER percent done: " << percent++ << "%";
      progress->flush();
    }
    Sample sample = sampler.NextSample();

    WriteSample( outFile, sample );
  }
  if (progress != NULL) {
    (*progress) << "\r                          \r";
    progress->flush();
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
