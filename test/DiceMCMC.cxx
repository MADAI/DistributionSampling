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

#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>
#include "time.h"

#include "MarkovChainMonteCarloSampler.h"
#include "DiceModel.h"

int main(int argc, char ** argv){
  srand(time(NULL));
  if(argc != 2) {
    std::cerr << "Useage:\n\t " << argv[0] << " info_dir_path\n\n"
    "where info_dir_path is the path to the "
    "directory containing all of the configuration "
    "files needed to run the mcmc.\n\n";
    return EXIT_FAILURE;
  }
  std::string info_dir(argv[1]);
  madai::DiceModel d_model;
  d_model.LoadConfigurationFile(info_dir);
  if(!d_model.good()){
    std::cerr << "Something is wrong with the model\n\n";
    return EXIT_FAILURE;
  }
  
  madai::MarkovChainMonteCarloSampler run(&d_model, info_dir);
  std::vector< madai::Parameter > const * parameters = &(d_model.GetParameters());
  for(int i = 0; i < parameters->size(); i++)
    run.ActivateParameter( (*parameters)[i].m_Name );
    
  madai::Trace trace(info_dir, "default" );
  if(run.m_BurnIn == 0){
    trace.add(run.m_InitialTheta);
  }
  
  for(int j = 0; j < d_model.GetNumberOfParameters(); j++)
  run.m_ParameterValues.push_back(0);
  
  run.m_AcceptCount = 0;
  for(run.m_IterationNumber = 1; run.m_IterationNumber < trace.m_MaxIterations; run.m_IterationNumber++){
    run.NextSample(&trace);
  }
  if(trace.m_CurrentIteration!=0){
    trace.WriteOut(d_model.GetParameters());
  }
  trace.MakeTrace();
  
  std::cerr << "Done Successfully." << std::endl;

  return EXIT_SUCCESS;
}
