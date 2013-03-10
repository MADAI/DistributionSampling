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

#ifndef madai_GaussianProcessEmulatedModel_h_included
#define madai_GaussianProcessEmulatedModel_h_included

#include "Model.h"
#include "GaussianProcessEmulator.h"

#include <string>
#include <vector>

namespace madai {

/** \class GaussianProcessEmulatedModel
 *
 * Interface to EmuPlusPlus.h */
class GaussianProcessEmulatedModel : public Model {
  public:

  /**
   * constructor
   */
  GaussianProcessEmulatedModel();

  /**
   * destructor
   */
  virtual ~GaussianProcessEmulatedModel();

  /**
   * Loads a model emulator from a file.
   */
  virtual ErrorType LoadConfigurationFile( const std::string fileName );

  /**
   * Get the scalar outputs from the model evaluated at point
   * \c parameters.
   */
  virtual ErrorType GetScalarOutputsAndCovariance(
      const std::vector< double > & parameters,
      std::vector< double > & scalars,
      std::vector< double > & scalarCovariance) const;

  /**
   * Get the scalar outputs from the model evaluated at point
   * \c parameters.
   */
  virtual ErrorType GetScalarOutputs( const std::vector< double > & parameters,
                                      std::vector< double > & scalars ) const;

  /**
   * METHOD_NOT_IMPLEMENTED
   */
  virtual Model::ErrorType GetLikeAndPrior(
    const std::vector<double>&, double&, double&) const {
    return METHOD_NOT_IMPLEMENTED;
  }

private:
  GaussianProcessEmulator m_GPME;

}; // end GaussianProcessEmulatedModel

} // end namespace madai

#endif // madai_GaussianProcessEmulatedModel_h_included
