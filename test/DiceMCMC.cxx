#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>
#include "time.h"

#include "MCMCRun.h"
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
  
  madai::MCMCRun run(&d_model, info_dir);
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
    run.NextIteration(&trace);
  }
  if(trace.m_CurrentIteration!=0){
    trace.WriteOut(d_model.GetParameters());
  }
  trace.MakeTrace();
  
  if(run.m_CreateTrace){
    run.m_Visualizer->FinalTrace(&trace);
  }
  
  std::cerr << "Done Successfully." << std::endl;

  return EXIT_SUCCESS;
}
