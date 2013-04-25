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

#ifndef madai_GaussianProcessEmulatorSingleFileWriter_h_included
#define madai_GaussianProcessEmulatorSingleFileWriter_h_included

#include <istream>


namespace madai {

// Forward declarations
class GaussianProcessEmulator;

class GaussianProcessEmulatorSingleFileWriter {
public:
  GaussianProcessEmulatorSingleFileWriter();
  ~GaussianProcessEmulatorSingleFileWriter();

  /**
    Writes current state to file.  \returns true on success. */
  bool Write(GaussianProcessEmulator * gpe,std::ostream & output) const;
  /**
    Writes the model data to a file. \returns true on success. */
  bool WriteModelData( GaussianProcessEmulator * gpe, std::ostream & output ) const;
  /**
    Writes current state of PCADecomposition to file. */
  bool WritePCA( GaussianProcessEmulator * gpe, std::ostream & output) const;

  /**
     Writes current state to file.  \returns true on sucess. */
  bool PrintThetas( GaussianProcessEmulator * gpe, std::ostream & output) const;

};

} // end namespace madai

#endif // madai_GaussianProcessEmulatorSingleFileWriter_h_included

