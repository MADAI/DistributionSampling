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

#ifndef madai_GaussianProcessEmulatorSingleFileReader_h_included
#define madai_GaussianProcessEmulatorSingleFileReader_h_included

#include <istream>


namespace madai {

// Forward declarations
class GaussianProcessEmulator;

class GaussianProcessEmulatorSingleFileReader {
public:
  GaussianProcessEmulatorSingleFileReader();
  ~GaussianProcessEmulatorSingleFileReader();

  /**
    This loads a fully-ready-to-go GPEM
    \returns true on success. */
  bool Load(GaussianProcessEmulator * gpe, std::istream & input);

  /**
    This takes an empty GPEM and loads training data.
    \returns true on success. */
  bool LoadTrainingData(GaussianProcessEmulator * gpe, std::istream & input);

  /**
    This takes an empty GPEM and loads training data.
    \returns true on success. */
  bool LoadTrainingData(GaussianProcessEmulator * gpe, std::string TopDirectory);

  /**
    This takes a GPEM and loads PCA data.
    \returns true on success. */
  bool LoadPCA(GaussianProcessEmulator * gpe, std::string TopDirectory);

  /**
    This takes a GPEM and loads the emulator specific
    data (submodels with their thetas).
    \returns true on success. */
  bool LoadEmulator(GaussianProcessEmulator * gpe, std::string TopDirectory);
  

};

} // end namespace madai

#endif // madai_GaussianProcessEmulatorSingleFileReader_h_included
