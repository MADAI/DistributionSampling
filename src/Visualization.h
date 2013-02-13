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

#ifndef __Visualization_h__
#define __Visualization_h__


#include "Trace.h"

#include <deque>


namespace madai {


class MCMCRun;

/** \class Visualization
 *
 * Class for generating a visualization from an MCMC run. */
class Visualization {
public:
  Visualization(MCMCRun * mcmc_in);
  ~Visualization();

  void operator() (const std::string& command);
  void UpdateTraceFig(Trace* ThetaOutsList);
  void FinalTrace(Trace* ThetaOutsList);

private:

  std::string              m_GNUPlotTerm;
  std::string              m_GNUPlotStyle;
  int                      m_HighestItnReadIn;
  bool                     m_MovingWindow;
  bool                     m_DensityPlot;
  MCMCRun*                 m_MCMC;
  std::string*             m_ParamValues;
  std::string              m_Header;
  deque<std::string>*      m_DequeParameterValues;
  FILE*                    m_GNUPlotPipe;
  FILE*                    m_GNUPlotMultipipe;
  std::vector<std::string> m_DensityPlotFileNames;
  std::vector<std::string> m_DensityPlotCommands;
  /**
   * The Densities array holds the density plot data. In principle the first two indices
   * should be NUM_PARAMS, and NUM_PARAMS-1, but I don't know how to do that here. The
   * last two are the number of bins.
   **/
  int m_Densities[7][7][100][100];
  int m_Bins;
};

}

#endif // __Visualization_h
