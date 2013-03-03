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
#include <string>
#include <vector>

#include "MarkovChainMonteCarloSampler.h"
#include "TestModel.h"
#include "VectorUtilities.h"

/***
 * Idea for a regression test: Do a run with the MCMC sampler with a
 * fixed seed. Each run should be identical with a fixed seed since
 * the sequence of random numbers from rand() will always be the same
 * for the same seed. If the trace changes between runs, then
 * something is wrong with the MCMC code.
 ***/


int main(int argc, char ** argv) {

  if (argc != 3) {
    std::cout << "Usage: " << argv[0]
	      << " info_dir_path comparison_trace\n\n"
      "where info_dir_path is the path to the "
      "directory containing all of the configuration "
      "files needed for the MarkovChainMonteCarloSampler class and comparison_trace holds the "
      "expected output from the MarkovChainMonteCarloSampler.\n\n";
    return EXIT_FAILURE;
  }

  srand( 100 );
  
  std::string info_dir(argv[1]);
  madai::TestModel model;
  model.LoadConfiguration(info_dir);
  if ( !model.IsReady() ) {
    std::cerr << "Something is wrong with the model\n";
    return EXIT_FAILURE;
  }

  std::string comparison_trace_filename( argv[2] );

  // Set up the sampler
  madai::MarkovChainMonteCarloSampler sampler(&model, info_dir);
  
  for ( int i = 0; i < model.GetNumberOfParameters(); i++ ) {
    sampler.ActivateParameter( i );
  }

  // Set up the trace
  madai::Trace trace;
  
  sampler.m_AcceptCount = 0;
  for ( int i = 0; i < 100; ++i ) {
    sampler.NextSample( &trace );
  }

  std::vector< madai::Parameter > parameters = model.GetParameters();

  std::string trace_file_name( "MarkovChainMonteCarloSamplerTest.csv" );
  if ( !trace.WriteCSVFile( trace_file_name, parameters ) ) {
    std::cerr << "Could not write file '" << trace_file_name << "'" << std::endl;
    return EXIT_FAILURE;
  }
  
  // At this point the trace for the new sampler has been written.
  // Now we want to compare it to a reference trace.
  
  madai::Trace referenceTrace;
  if ( !referenceTrace.ImportCSVFile( comparison_trace_filename,
				      model.GetNumberOfParameters(),
				      model.GetNumberOfScalarOutputs() ) ) {
    std::cerr << "Could not import reference CSV file '" << comparison_trace_filename
	      << "'" << std::endl;
    return EXIT_FAILURE;
  }
  
  for ( int i = 0; i < trace.GetSize(); ++i ) {
    madai::TraceElement referenceElement = referenceTrace[i];
    madai::TraceElement traceElement = trace[i];

    if ( referenceElement.m_ParameterValues != traceElement.m_ParameterValues ) {
      std::cerr << "Parameter values in read trace do not match at entry " << i << std::endl;
      std::cerr << "Got " << referenceElement.m_ParameterValues << ", should have been "
		<< traceElement.m_ParameterValues << std::endl;
      return EXIT_FAILURE;
    }

    if ( referenceElement.m_OutputValues != traceElement.m_OutputValues ) {
      std::cerr << "Output values in read trace do not match at entry " << i << std::endl;
      std::cerr << "Got " << traceElement.m_OutputValues << ", should have been "
		<< referenceElement.m_OutputValues << std::endl;
      return EXIT_FAILURE;
    }

    if ( referenceElement.m_LogLikelihood != traceElement.m_LogLikelihood ) {
      std::cerr << "Log likelihood in read trace does not match at entry " << i << std::endl;
      std::cerr << "Got " << traceElement.m_LogLikelihood << ", should have been "
		<< referenceElement.m_LogLikelihood << std::endl;
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}

