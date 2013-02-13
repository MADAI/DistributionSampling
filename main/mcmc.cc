/*=========================================================================
 *
 *  Copyright (c) 2010-2012 The University of North Carolina at Chapel Hill
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

#include "MCMCRun.h"
#include "RHICModel.h"

int main(int argc, char ** argv){
  srand(time(NULL));
  if(argc != 2) {
    std::cerr << 
    "Useage:\n\t mcmc info_dir_path\n\n"
    "where info_dir_path is the path to the "
    "directory containing all of the configuration "
    "files needed to run the mcmc.\n\n";
    return 0;
  }
  std::string info_dir(argv[1]);
  madai::RHICModel m_model;
  m_model.LoadConfiguration(info_dir);
  if(!(m_model.IsReady())){
    std::cerr << "Something is wrong with the model\n\n";
    return 0;
  }
  
  madai::MCMCRun run(&m_model, info_dir);
    
  std::vector<madai::Parameter> const * parameters = &(m_model.GetParameters());
  for(int i = 0; i < parameters->size(); i++)
    run.ActivateParameter( (*parameters)[i].m_Name );
   
  madai::Trace trace(info_dir,"default");
  if(run.m_BurnIn == 0){
    trace.add(run.m_InitialTheta);
  }
    
  for(int j=0; j<m_model.GetNumberOfParameters(); j++)
    run.m_ParameterValues.push_back(0);
    
	run.m_AcceptCount = 0;
	for(run.m_IterationNumber = 1; run.m_IterationNumber < trace.m_MaxIterations; run.m_IterationNumber++){
		run.NextIteration(&trace);
  }
  if(trace.m_CurrentIteration!=0){
    trace.WriteOut(m_model.GetParameters());
  }
	trace.MakeTrace();
    
	double ratio = (double)run.m_AcceptCount/((double)trace.m_MaxIterations-(double)run.m_BurnIn);
  std::cout << "Accepts: " << run.m_AcceptCount << std::endl;
  std::cout << "Iterations-Burn in: " << trace.m_MaxIterations-run.m_BurnIn << std::endl;
  std::cout << "Acceptance ratio: " << ratio << std::endl;
  printf("-------- Best Parameter Set, likelihood=%g -------------\n",run.m_BestLikelihood);
  std::cout << "This parameter set contains " << parameters->size() << " parameters." << std::endl;
  for(int i=0;i<parameters->size();i++){
    std::cout << (*parameters)[i].m_Name << ":\t" << run.m_BestParameterSet[i] << std::endl;
  }
  
  std::cout << "Done Successfully." << std::endl;
  return 0;
}
