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

#ifndef __Distribution_h__
#define __Distribution_h__

#include <gsl/gsl_blas.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_permutation.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_vector.h>

#include <set>

#include "parametermap.h"
#include "Model.h"

namespace madai {
    
class Model;

/** \class Distribution
 *
 * Base class for distributions. */
class Distribution {
public:
  Distribution();
  virtual ~Distribution();
  
  double Normal(double x, double mu, double sigma);
  double IntegratedNormal(double x, double mu, double sigma, double data_sigma);
  double Gaussian(double x, double mu, double sigma);
  double Gaussian(gsl_vector x, gsl_vector mu, gsl_matrix sigma);
  double Gaussian(gsl_vector x, gsl_vector mu, gsl_matrix sigma, gsl_matrix data_sigma);
  double Log_MVNormal(gsl_vector x, gsl_vector mu, gsl_matrix sigma);
  double MVNormal(gsl_vector x, gsl_vector mu, gsl_matrix sigma);
  double LogNormal(double x, double mu, double sigma);

protected:

  Model*        m_Model;
  gsl_rng*      m_RandNumGen;
  bool          m_SepMap;
  bool          m_Timing;
  bool          m_Verbose;
  bool          m_Debug;
  parameterMap* m_ParameterMap;
};

/** ---------------------------------------- */

// Proposal isn't currently being used (transfered functions to MCMCRun)
class ProposalDistribution : public Distribution {
public:
  ProposalDistribution(Model *m_Model);
  std::vector<double> Iterate(std::vector<double>& current,
                              double& scale);
  virtual double Evaluate(std::vector<double> Theta1, 
                          std::vector<double> Theta2, 
                          double scale);
    
protected:
    
  bool                  m_SymmetricProposal;
  bool                  m_RescaledMethod;
  double                m_MinScale;
  double                m_MaxScale;
  double                m_Prefactor;
  double                m_Scale;
  std::vector<double>   m_MixingStdDev;
  std::set<std::string> m_ActiveParameters;
    
  int FindParam(std::string param_name);
};


} // namespace madai

#endif
