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

#ifndef __MarkovChainMonteCarloSampler_h__
#define __MarkovChainMonteCarloSampler_h__

#include <map>
#include <string>
#include <vector>

#include <gsl/gsl_randist.h>

#include "parametermap.h"
#include "random.h"

#include "Model.h"
#include "MultiModel.h"
#include "Sampler.h"
#include "Trace.h"


namespace madai {

class Model;
class Trace;
class TraceElement;


/* \class MarkovChainMonteCarloSampler
 *
 * MarkovChainMonteCarloSampler loads in parameters for running a
 * monte carlo analysis and provides a method for taking steps in
 * parameter space, defined by said parameters.
 */
class MarkovChainMonteCarloSampler : public Sampler {
public:
  MarkovChainMonteCarloSampler(const Model *in_model, const std::string info_dir);
  ~MarkovChainMonteCarloSampler();

  void NextSample( Trace *trace );

  std::vector<double> GetRandomTheta0(int seed);
  std::vector<double> GetTheta0FromFile();

  std::string         m_DirectoryName;
  parameterMap        m_LocalParameterMap;
  std::vector<double> m_BestParameterSet;
  std::vector<double> m_ParameterValues;
  std::vector<double> m_InitialTheta;
  int                 m_BurnIn;
  bool                m_RandomTheta0;
  bool                m_VizTrace;
  bool                m_Quiet;
  bool                m_RescaledTrace;
  bool                m_AppendTrace;
  bool                m_LogLike;
  bool                m_LogPrior;
  bool                m_LogProposal;
  bool                m_CreateTrace;
  bool                m_Debug;

  CRandom*            m_RandomNumber;

  std::vector<double> m_CurrentParameters;
  double              m_LikelihoodCurrent;
  double              m_LikelihoodNew;
  double              m_PriorCurrent;
  double              m_PriorNew;
  double              m_ProposalCurrent;
  double              m_ProposalNew;
  double              m_BestLikelihood;
  double              m_ScaleNew;
  double              m_ScaleCurrent;
  int                 m_AcceptCount;
  int                 m_VizCount;
  int                 m_IterationNumber;

  // Taking a step and calculating the probability of the step
  void LoadStepParameters();
  std::vector<double> TakeStep( std::vector<double>& current,
                                double& scale);
  double EvaluateProposal( std::vector<double> Theta1,
                           std::vector<double> Theta2,
                           double scale );

  gsl_rng*            m_RandNumGen;
  bool                m_SepMap;
  bool                m_Timing;
  bool                m_SymmetricProposal;
  bool                m_RescaledMethod;
  bool                m_FlatStep;
  double              m_Prefactor;
  double              m_MinScale;
  double              m_MaxScale;
  double              m_Scale;
  double              m_Offset;
  std::vector<double> m_MixingStdDev;
  parameterMap*       m_StepParameterMap;
};

} // end namespace madai

#endif // __MarkovChainMonteCarloSampler_h__
