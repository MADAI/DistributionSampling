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

#ifndef madai_Sample_h_included
#define madai_Sample_h_included

#include <string>
#include <vector>


namespace madai {


/** \class Sample
 *
 * An individual sample from a distribution. A sample contains
 * parameter values (the point in parameter space where the sample was
 * taken), output values (obtained by evaluating the Model at the
 * sample point, and the relative log likelihood of the sample.
 */
class Sample {
public:
  //@{
  /**
   * If LogLikelihood is not set, it defaults to 0.0. If
   * outputValues is not specified, it defaults to an empty
   * vector. If parameterValues is not specified, it defaults to an
   * empty vector. If likelihoodErrorGradient is not specified, it
   * defaults to an empty vector.
   */
  Sample( const std::vector< double > & parameterValues,
    const std::vector< double > & outputValues,
    double LogLikelihood,
    const std::vector< double > logLikelihoodValueGradient,
    const std::vector< double > logLikelihoodErrorGradient);

  Sample( const std::vector< double > & parameterValues,
    const std::vector< double > & outputValues,
    double LogLikelihood );

  Sample( const std::vector< double > & parameterValues,
    const std::vector< double > & outputValues );

  Sample( const std::vector< double > & parameterValues );

  Sample();
  //@}

  /** Clear all the parameter values, output values, and comments, and
   * set the log likelihood to 0.0. */
  void Reset();

  /** Returns true if there are any parameter values, output values,
   * or comments. */
  bool IsValid() const;

  /** A set of Parameters values from a model. */
  std::vector< double > m_ParameterValues;

  /** The model output that correspond to m_ParameterValues in some model. */
  std::vector< double > m_OutputValues;

  /** The gradient dLL/dobservable for each observable parameter. */
  std::vector< double > m_LogLikelihoodValueGradient;

  /** The gradient sigma_observable*dLL/dsigma_observable for each observable parameter. */
  std::vector< double > m_LogLikelihoodErrorGradient;

  /**
   * Given some set of field observations, the log likelihood is the
   * relative log likelihood that m_ParameterValues is the ground
   * truth.  this->m_LogLikelihood is (ln(C * Likelihood)) for some
   * unknown constant C.
  */
  double m_LogLikelihood;

  /** Comments may be used to store human-readable comments or record
   *  changes to state, such as changing an optimizer type, which
   *  parameters are activated, etc.. */
  std::vector< std::string > m_Comments;

  /**
   * Provide this operator so that we can do:
   * \code
   *  void SortSamples(std::vector< Sample > & s) {
   *    std::sort(s.begin(),s.end());
   *  }
   * \endcode
   */
  bool operator<(const Sample & rhs) const;

  /**
   * Equality operator so we can check if a sampler returned the same
   * sample multiple times.
   */
  bool operator==(const Sample & rhs) const;

}; // class Sample

/**
 Operator for printing to an output stream.
*/
std::ostream & operator<<(std::ostream & os, const Sample & sample);


} // end namespace madai

#endif // madai_Sample_h_included
