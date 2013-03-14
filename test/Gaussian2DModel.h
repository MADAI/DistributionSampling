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

#ifndef madai_Gaussian2DModel_h_included
#define madai_Gaussian2DModel_h_included

#include "Model.h"


namespace madai {

/** \class Gaussian2DModel
 *
 * A test model used internally. */
class Gaussian2DModel : public Model {
public:
  Gaussian2DModel();
  ~Gaussian2DModel() {};

  /** Loads a configuration from a file. For this model, the
   * configuration file is a text file containing four numbers
   * separated by whitespace (space, tab, newline). The numbers are
   * the mean in x, mean in y, standard deviation in x and standard
   * deviation in y of the 2D Gaussian function. */
  ErrorType LoadConfigurationFile( const std::string fileName );
    
  /** Get the scalar outputs from the model evaluated at x. */
  ErrorType GetScalarOutputs( const std::vector< double > & parameters,
                              std::vector< double > & scalars ) const;

  /** Get both scalar values and the gradient of the parameters. */
  ErrorType GetScalarAndGradientOutputs( const std::vector< double > & parameters,
                                         const std::vector< bool > & activeParameters,
                                         std::vector< double > & scalars,
                                         std::vector< double > & gradient) const;

  /** Override this to ignore any observed values.
   *
   * This is a test model where we are setting the observed values
   * internally, so we preclude client code from setting observed
   * values. */
  virtual ErrorType SetObservedScalarValues(
    const std::vector< double > & observedScalarValues);

  /** Override this to ignore any oberved value covariance.
   *
   * This is a test model where we are setting the observed values
   * internally, so we preclude client code from setting observed
   * covariance. */
  virtual ErrorType SetObservedScalarCovariance(
    const std::vector< double > & observedScalarCovariance);

  /** Get the x and y means */
  void GetMeans( double & MeanX, double & MeanY ) const;

  /** Get the standard deviations in x and y */
  void GetDeviations( double & DvX, double & DevY ) const;

protected:
  /** Means in x and y */
  //@{
  double m_MeanX;
  double m_MeanY;
  //@}

  /** Standard deviations in x and y */
  //@{
  double m_StandardDeviationX;
  double m_StandardDeviationY;
  //@}

  /** Computes the partial derivatives of x and y */
  //@{
  double PartialX( double x, double value ) const;
  double PartialY( double y, double value ) const;
  //@}

}; // end class Gaussian2DModel

} // end namespace madai

#endif // madai_Gaussian2DModel_h_included
