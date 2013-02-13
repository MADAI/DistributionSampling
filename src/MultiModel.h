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

#ifndef __MultiModel_h__
#define __MultiModel_h__

#include <vector>
#include <string>
#include <map>
#include <math.h>
#include <iostream>
#include <fstream>
#include "parametermap.h"
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <time.h>
#include <sys/stat.h>

#include "Model.h"
#include "Distribution.h"

namespace madai {

class LikelihoodDistribution;
class PriorDistribution;

/** \class MultiModel
 *
 * This class provides a method for reading in a model configuration
 * defined with more than one file.
 *
 * Other functions are specified in subclasses.
 */
class MultiModel : public Model {
public:
  MultiModel();
  virtual ~MultiModel();

  /**
   * Load a configuration from multiple files. The format of these
   * files is defined by the following functions
   **/
  virtual ErrorType LoadConfigurationFile( const std::string fileName );
  virtual ErrorType LoadConfiguration(const std::string info_dir);

  /**
   * Calculation of the observalues and their variances
   **/
  virtual ErrorType GetScalarOutputs( const std::vector< double > & parameters,
                                      std::vector< double > & scalars ) const = 0;

  // For interaction with the MCMC
  /** Get the likelihood and prior at the point parameters in parameter space. */
  virtual ErrorType GetLikeAndPrior( const std::vector< double > & parameters,
                                     double & Like,
                                     double & Prior ) const = 0;

  virtual ErrorType LoadDistributions() = 0;

  // Put these here so that RHICModel could use them for LoadProcess();
  void discard_line(std::FILE* fp);
  bool discard_comments(std::FILE* fp, char comment_character);
  void eat_whitespace(std::istream& i);
  void eat_whitespace(std::FILE* fp);
  bool discard_comments(std::istream& i, char comment_character);

protected:
  unsigned int m_NumberOfParameters;
  unsigned int m_NumberOfOutputs;

  std::vector< bool >     m_LogParam;
  std::string             m_ModelType;
  std::string             m_ParameterFile;
  std::string             m_Optimizer;
  LikelihoodDistribution* m_Likelihood;
  PriorDistribution*      m_Prior;
  bool                    m_PrescaledParams;
  bool                    m_UseEmulator;
  bool                    m_ProcessPipe;

}; // end class MultiModel

} // end namespace madai

#endif // __MultiModel_h
