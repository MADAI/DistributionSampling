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

namespace madai {

class Sampler;
class Model;

/**
 * \class SamplerCSVWriter
 *
 * \todo Document this class.
 */
class SamplerCSVWriter {
public:
  /**
     Execute Sampler and save trace to file.
     If progress is not NULL, will print out a progrss bar.
   */
  static int GenerateSamplesAndSaveToFile(
    Sampler * sampler,
    Model * model,
    std::ostream & outFile,
    int NumberOfSamples,
    int NumberOfBurnInSamples=0,
    bool UseEmulatorCovariance=true,
    std::ostream * progress=NULL);

}; // end SamplerCSVWriter

} // end namespace madai

#endif // madai_SamplerCSVWriter_h_included
