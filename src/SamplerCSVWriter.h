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

#ifndef madai_SamplerCSVWriter_h_included
#define madai_SamplerCSVWriter_h_included

#include <iostream>
#include <string>
#include <vector>

namespace madai {

class Parameter;
class Sample;
class Sampler;
class Model;

/**
 * \class SamplerCSVWriter
 *
 * This class takes a Sampler and a Model and writes a requested
 * number of Samples in comma-separated value file format to an output
 * stream.
 */
class SamplerCSVWriter {
public:
  /**
   * Execute Sampler and save Samples to a comma-separated value file.
   *
   * If progress is not NULL, will print out a progress bar to that
   *  output stream.
   */
  static int GenerateSamplesAndSaveToFile(
    Sampler & sampler,
    Model & model,
    std::ostream & outFile,
    int NumberOfSamples,
    int NumberOfBurnInSamples=0,
    bool UseEmulatorCovariance=true,
    bool WriteLogLikelihoodGradients=false,
    std::ostream * progress=NULL);

  /**
   * Writes the header of the CSV file. This consists of the parameter
   * names, the output names, and the log likelihood. */
  static void WriteHeader( std::ostream & o,
                           const std::vector< Parameter > & parameters,
                           const std::vector< std::string > & outputs,
                           bool WriteLogLikelihoodGradients = false);

  /**
   * Writes one sample to the output stream.
   */
  static void WriteSample( std::ostream & out, const Sample & sample,
                           bool WriteLogLikelihoodGradients = false);

}; // end SamplerCSVWriter

} // end namespace madai

#endif // madai_SamplerCSVWriter_h_included
