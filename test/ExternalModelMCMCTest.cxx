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

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "ExternalModel.h"
#include "GaussianDistribution.h"
#include "MetropolisHastingsSampler.h"
#include "Trace.h"
#include "UniformDistribution.h"


/**
 * Test case for madai::ExternalModel and
 * madai::MetropolisHastingsSampler classes.
 */
int main(int argc, char ** argv) {

  if (argc < 2) {
    std::cerr <<
      "Usage: " << argv[0] << " external_executable\n\n"
      "where external_executable is suitable for the\n"
      "madai::ExternalModel class.\n\n";
    return EXIT_FAILURE;
  }

  std::string executable( argv[1] );
  madai::ExternalModel externalModel;

  // This executable does not require any arguments
  std::vector< std::string > arguments;

  externalModel.StartProcess( executable, arguments );
  if ( !externalModel.IsReady() ) {
    std::cerr << "Something is wrong with the external model\n";

    return EXIT_FAILURE;
  }

  // Let's check that the parameters are defined as expected. This
  // assumes that the external process is the file in
  // DistributionSampling/test/ExternalModelMCMCTest/ExampleProcess.py
  const std::vector< madai::Parameter > & parameters =
    externalModel.GetParameters();
  if ( parameters.size() != 3 ) {
    std::cerr << "Only " << parameters.size()
              << " parameters retrieved from " << argv[0] << ", expected 3.";
    return EXIT_FAILURE;
  }

  madai::Distribution * distribution = NULL;
  madai::UniformDistribution * uniformDistribution = NULL;
  madai::GaussianDistribution * gaussianDistribution = NULL;

  if ( parameters[0].m_Name != "param_0" ) {
    std::cerr << "Name of parameter 0 is '" << parameters[0].m_Name
              << "', expected 'param_0'" << std::endl;
    return EXIT_FAILURE;
  }

  distribution = parameters[0].m_PriorDistribution;
  uniformDistribution = dynamic_cast< madai::UniformDistribution * >( distribution );
  if ( !uniformDistribution ) {
    std::cerr << "Prior distribution for parameter 0 is not a "
              << "UniformDistribution, but should be\n";
    return EXIT_FAILURE;
  } else {
    if ( uniformDistribution->GetMinimum() != -1.0 ) {
      std::cerr << "Prior distribution minimum for parameter 0 was "
                << uniformDistribution->GetMinimum() << ", should have been "
                << -1.0 << "\n";
      return EXIT_FAILURE;
    }
    if ( uniformDistribution->GetMaximum() != 2.0 ) {
      std::cerr << "Prior distribution maximum for parameter 0 was "
                << uniformDistribution->GetMaximum() << ", should have been "
                << 2.0 << "\n";
      return EXIT_FAILURE;
    }
  }

  if ( parameters[1].m_Name != "param_1" ) {
    std::cerr << "Name of parameter 1 is '" << parameters[1].m_Name
              << "', expected 'param_1'" << std::endl;
    return EXIT_FAILURE;
  }

  distribution = parameters[1].m_PriorDistribution;
  uniformDistribution = dynamic_cast< madai::UniformDistribution * >( distribution );
  if ( !uniformDistribution ) {
    std::cerr << "Prior distribution for parameter 1 is not a "
              << "UniformDistribution, but should be\n";
    return EXIT_FAILURE;
  } else {
    if ( uniformDistribution->GetMinimum() != -2.1 ) {
      std::cerr << "Prior distribution minimum for parameter 1 was "
                << uniformDistribution->GetMinimum() << ", should have been "
                << -2.1 << "\n";
      return EXIT_FAILURE;
    }
    if ( uniformDistribution->GetMaximum() != 3.7 ) {
      std::cerr << "Prior distribution maximum for parameter 1 was "
                << uniformDistribution->GetMaximum() << ", should have been "
                << 3.7 << "\n";
      return EXIT_FAILURE;
    }
  }

  if ( parameters[2].m_Name != "param_2" ) {
    std::cerr << "Name of parameter 2 is '" << parameters[2].m_Name
              << "', expected 'param_2'" << std::endl;
    return EXIT_FAILURE;
  }

  distribution = parameters[2].m_PriorDistribution;
  gaussianDistribution = dynamic_cast< madai::GaussianDistribution * >( distribution );
  if ( !gaussianDistribution ) {
    std::cerr << "Prior distribution for parameter 2 is not a "
              << "GaussianDistribution, but should be\n";
    return EXIT_FAILURE;
  } else {
    if ( gaussianDistribution->GetMean() != -5.0 ) {
      std::cerr << "Prior distribution mean for parameter 2 was "
                << gaussianDistribution->GetMean() << ", should have been "
                << -5.0 << "\n";
      return EXIT_FAILURE;
    }
    if ( gaussianDistribution->GetStandardDeviation() != 3.1 ) {
      std::cerr << "Prior distribution standard deviation for parameter 2 was "
                << gaussianDistribution->GetStandardDeviation()
                << ", should have been " << 3.1 << "\n";
      return EXIT_FAILURE;
    }
  }

  // Now check the outputs
  std::vector< std::string > outputNames = externalModel.GetScalarOutputNames();
  if ( outputNames.size() != 1 ) {
    std::cerr << "Number of outputs is " << outputNames.size() << ", should be 1\n";
    return EXIT_FAILURE;
  }

  if ( outputNames[0] != "output" ) {
    std::cerr << "Output 0 is named '" << outputNames[0]
              << "', should have been 'output'" << std::endl;
    return EXIT_FAILURE;
  }

  madai::MetropolisHastingsSampler simple_mcmc;
  simple_mcmc.SetModel( &externalModel );

  simple_mcmc.SetStepSize(0.1);

  int t = externalModel.GetNumberOfScalarOutputs();
  std::vector< double > observedScalarValues;
  for(int i = 0; i < t; ++i)
    observedScalarValues.push_back(0.2);
  externalModel.SetObservedScalarValues(observedScalarValues);
  std::vector< double > observedScalarCovariance(t * t, 0.0);
  for(int i = 0; i < t; ++i)
    observedScalarCovariance[i + (t * i)] = 0.05;
  externalModel.SetObservedScalarCovariance(observedScalarCovariance);

  madai::Trace trace;
  unsigned int numberIter = 500;
  for (unsigned int count = 0; count < numberIter; count ++) {
    madai::Sample sample = simple_mcmc.NextSample();
    trace.Add( sample );
  }

  // Stop the external process
  externalModel.StopProcess();

  trace.WriteCSVOutput( std::cout,
                   externalModel.GetParameters(),
                   externalModel.GetScalarOutputNames() );

  return EXIT_SUCCESS;
}
