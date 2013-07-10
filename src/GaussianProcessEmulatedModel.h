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

#ifndef madai_GaussianProcessEmulatedModel_h_included
#define madai_GaussianProcessEmulatedModel_h_included

#include "Model.h"

#include <string>
#include <vector>

namespace madai {

class GaussianProcessEmulator;

/** \class GaussianProcessEmulatedModel
 *
 * This class presents the GaussianProcessEmulator class as a Model
 * that can be used by instances of the Sample class. */
class GaussianProcessEmulatedModel : public Model {
  public:

  GaussianProcessEmulatedModel();
  virtual ~GaussianProcessEmulatedModel();

  /**
   * Set the gaussian process emulator.
   *
   * \param gpe The instance of the GaussianProcessEmulator this
   *            object adapts.
   */
  virtual ErrorType SetGaussianProcessEmulator( GaussianProcessEmulator & gpe );

  /**
   *  Returns a const reference to internal data for debugging
   *  purposes.
   */
  const GaussianProcessEmulator & GetGaussianProcessEmulator() const;

  /**
   * Get the scalar outputs from the model evaluated at point
   * \c parameters.
   *
   * \param parameters Parameter values where the model should be evaluated.
   * \param scalars    Storage for scalar values returned by this method.
   * \param scalarCovariance Storage for the covariance of the model
   *                         at this point in parameter space.
   */
  virtual ErrorType GetScalarOutputsAndCovariance(
      const std::vector< double > & parameters,
      std::vector< double > & scalars,
      std::vector< double > & scalarCovariance) const;

  /** Get the scalar outputs from the model evaluated at x
   *
   * \param parameters Parameter values where the Model should be
   * evaluated. The length of this vector must equal the number of the
   * parameters in the Model.
   * \param scalars A vector to hold the scalar outputs from the Model
   * when evaluated at the point in parameter space specified by the
   * parameters argument. */
  virtual ErrorType GetScalarOutputs( const std::vector< double > & parameters,
                                      std::vector< double > & scalars ) const;

  /** Get both scalar outputs and the gradient of active parameters.
   *
   * The gradient output parameter will contain gradient components of
   * the log likelihood for only the active parameters. That is, the
   * first element in the vector will contain the gradient component
   * for the first active parameter, the second element in the vector
   * will contain the gradient component for the second active
   * parameter, and so on.
   *
   * \param parameters Point in parameter space where the Model should
   * be evaluated.
   * \param activeParameters List of parameters for which the gradient
   * should be computed.
   * \param scalars Output argument that will contain the scalars from
   * evaluating the Model.
   * \param gradient Output argument that will contain the gradient
   * components requested via the activeParameters vector.
   */
  virtual ErrorType GetScalarAndGradientOutputs(
    const std::vector< double > & parameters,
    const std::vector< bool > & activeParameters,
    std::vector< double > & scalars,
    std::vector< double > & gradient ) const;

  /**
   * Returns the combined training and observed covariance at point \c
   * x in the parameter space.
   *
   * \param x The point in the parameter space.
   */
  virtual bool GetConstantCovariance(std::vector< double > & x) const;

protected:
  /** The square root of the sum of the training data covariance and
   *  observation covariance. */
  std::vector< double > m_TrainingAndObservedCovariance;

private:
  /** The Gaussian process emulator. */
  GaussianProcessEmulator * m_GPE;

}; // end GaussianProcessEmulatedModel

} // end namespace madai

#endif // madai_GaussianProcessEmulatedModel_h_included
