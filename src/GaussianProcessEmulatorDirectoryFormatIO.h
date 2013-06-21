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

#ifndef madai_GaussianProcessEmulatorDirectoryFormatIO_h_included
#define madai_GaussianProcessEmulatorDirectoryFormatIO_h_included

#include <string>
#include <vector>


namespace madai {

// Forward declarations
class GaussianProcessEmulator;
class Parameter;

/** \class GaussianProcessEmulatorDirectoryFormatIO
 *
 * Initializes a GaussianProcessEmulator from the files in a specific
 *  directory structure. */
class GaussianProcessEmulatorDirectoryFormatIO {
public:
  GaussianProcessEmulatorDirectoryFormatIO();
  ~GaussianProcessEmulatorDirectoryFormatIO();

  /**
   Enable verbose output when reading. */
  //@{
  void SetVerbose( bool value );
  bool GetVerbose() const;
  //@}

  /**
    This takes an empty GaussianProcessEmulator and loads training data.
    \returns true on success. */
  bool LoadTrainingData(GaussianProcessEmulator * gpe,
                        std::string modelOutputDirectory,
                        std::string statisticalAnalysisDirectory,
                        std::string experimentalResultsFileName);

  /**
    This takes a GaussianProcessEmulator and loads principal component
    analysis data.
    \returns true on success. */
  bool LoadPCA( GaussianProcessEmulator * gpe,
                const std::string & statisticalAnalysisDirectory);

  /**
    Writes current state to file.  \returns true on success. */
  bool Write(GaussianProcessEmulator * gpe,std::ostream & output) const;

  /**
    Writes current state of PCADecomposition to file. */
  bool WritePCA( GaussianProcessEmulator * gpe, std::ostream & output) const;

  /**
     Writes current state to file.  \returns true on sucess. */
  bool PrintThetas( GaussianProcessEmulator * gpe, std::ostream & output) const;

  /**
    This takes a GPEM and loads the emulator specific
    data (submodels with their thetas).
    \returns true on success. */
  bool LoadEmulator( GaussianProcessEmulator * gpe,
                     const std::string & statisticalAnalysisDirectory);

  /**
    Parses a file describing the parameter prior distributions. */
  static bool ParseParameters( std::vector< madai::Parameter > & parameters,
                               int & numberParameters,
                               const std::string & statisticalAnalysisDirectory,
                               bool verbose );

  /**
    Parses a file describing the outputs of a model. */
  static bool ParseOutputs( std::vector< std::string > & outputNames,
                            int & numberOutputs,
                            const std::string & statisticalAnalysisDirectory,
                            bool verbose );

protected:
  /** If true, produce a lot of output about which files are being
   *  opened, which values are read, etc. to stdout. */
  bool m_Verbose;

};

} // end namespace madai

#endif // madai_GaussianProcessEmulatorDirectoryFormatIO_h_included
