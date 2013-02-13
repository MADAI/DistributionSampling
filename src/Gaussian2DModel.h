/*=========================================================================
 *
 *  Copyright The University of North Carolina at Chapel Hill
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

#ifndef __Gaussian2DModel_h__
#define __Gaussian2DModel_h__

#include "Model.h"


namespace madai {

/** \class Gaussian2DModel
 *
 * A simple example of a model. */
class Gaussian2DModel : public Model {
public:
  Gaussian2DModel();
  ~Gaussian2DModel() {};

  /** Loads a configuration from a file. For this model, the
   * configuration file is a text file containing four numbers
   * separated by whitespace (space, tab, newline). The numbers are
   * the mean in x, mean in y, standard deviation in x and standard
   * devaition in y of the 2D Gaussian function. */
  ErrorType LoadConfigurationFile( const std::string fileName );
    
  /** Get the valid range for the parameter at parameterIndex. */
  void GetRange( unsigned int parameterIndex, double range[2] );

  /** Get the scalar outputs from the model evaluated at x. */
  ErrorType GetScalarOutputs( const std::vector< double > & parameters,
                              std::vector< double > & scalars ) const;

  /** Get both scalar values and the gradient of the parameters. */
  ErrorType GetScalarAndGradientOutputs( const std::vector< double > & parameters,
                                         const std::vector< bool > & activeParameters,
                                         std::vector< double > & scalars,
                                         unsigned int outputIndex,
                                         std::vector< double > & gradient) const;
  // Not implemented yet.
  // Proposed function for interaction with the MCMC
  virtual ErrorType GetLikeAndPrior( const std::vector< double > & parameters,
                                     double & Like,
                                     double & Prior ) const;

  void GetMeans( double & MeanX, double & MeanY ) const;
  void GetDeviations( double & DvX, double & DevY ) const;

protected:
  double m_MeanX;
  double m_MeanY;
  double m_StandardDeviationX;
  double m_StandardDeviationY;

  double PartialX( double x, double value ) const;
  double PartialY( double y, double value ) const;

}; // end class Gaussian2DModel

} // end namespace madai

#endif // __Gaussian2DModel_h__
