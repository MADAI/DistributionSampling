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

#include "Gaussian2DModel.h"


int main(int argc, char *argv[])
{
  madai::Gaussian2DModel *model = new madai::Gaussian2DModel();

  // Test loading a configuration file.
  // TODO - change the file name when LoadConfigurationFile actually
  // parses a file.
  std::string fileName = "filename.txt";
  madai::Model::ErrorType error = model->LoadConfigurationFile( fileName );
  if ( error != madai::Model::NO_ERROR )
  {
    std::cerr << "Error while loading configuration file '"
              << fileName << "'" << std::endl;
    return EXIT_FAILURE;
  }

  // Check the number of parameters.
  if ( model->GetNumberOfParameters() != 2 )
  {
    std::cerr << "Wrong number of parameters. Expected 2, got "
              << model->GetNumberOfParameters() << std::endl;
    return EXIT_FAILURE;
  }

  // Check the parameter names.
  const std::vector< madai::Parameter > parameterDescriptions =
       model->GetParameters();
  if ( parameterDescriptions.size() != model->GetNumberOfParameters() )
  {
    std::cerr << "Wrong number of elements in parameter vector. Expected "
              << model->GetNumberOfParameters() << ", got "
              << parameterDescriptions.size() << std::endl;
    return EXIT_FAILURE;
  }

  if ( parameterDescriptions[0].m_Name != "X" )
  {
    std::cerr << "Parameter 0 has unexpected name '"
              << parameterDescriptions[0].m_Name
              << "', expected 'X'." << std::endl;
    return EXIT_FAILURE;
  }

  if ( parameterDescriptions[1].m_Name != "Y" )
  {
    std::cerr << "Parameter 1 has unexpected name '" << parameterDescriptions[1].m_Name
              << "', expected 'Y'." << std::endl;
    return EXIT_FAILURE;
  }

  // Test parameter ranges
  for ( unsigned int i = 0; i < 2; i++ )
  {
    double range[2];
    model->GetRange( 0, range );
    if ( range[0] != -DBL_MAX )
    {
      std::cerr << "Got parameter lower range of " << range[0]
                <<  ", expected " << -DBL_MAX << std::endl;
      return EXIT_FAILURE;
    }

    if ( range[1] != DBL_MAX )
    {
      std::cerr << "Got parameter upper range of " << range[1]
                <<  ", expected " << DBL_MAX << std::endl;
      return EXIT_FAILURE;
    }
  }

  // Test scalar output number and names.
  if ( model->GetNumberOfScalarOutputs() != 1 )
  {
    std::cerr << "Expected number of scalar outputs to be 1, got "
              << model->GetNumberOfScalarOutputs() << std::endl;
    return EXIT_FAILURE;
  }

  const std::vector< std::string > scalarOutputNames =
       model->GetScalarOutputNames();
  if ( scalarOutputNames.size() != model->GetNumberOfScalarOutputs() )
  {
    std::cerr << "Wrong number of elements in scalar output name vector. Expected "
              << model->GetNumberOfScalarOutputs() << ", got "
              << scalarOutputNames.size() << std::endl;
    return EXIT_FAILURE;
  }

  // Test scalar outputs.
  // TODO - actually check the scalar output values
  std::vector< double > parameters;
  parameters.push_back( 22.2 );
  parameters.push_back( -14.0 );
  
  std::vector< double > scalars;
  error = model->GetScalarOutputs( parameters, scalars );
  if ( error != madai::Model::NO_ERROR )
  {
    std::cerr << "Error encountered when computing scalar outputs."
              << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Model value for parameters [" << parameters[0] << ", "
            << parameters[1] << "]: [" << scalars[0] << "]" << std::endl;

  // Test scalar and gradient outputs
  std::vector< double > gradient;
  std::vector< bool > activeParameters( model->GetNumberOfParameters() );
  for (unsigned int i = 0; i < activeParameters.size(); i++)
  {
    activeParameters[i] = true;
  }
  error = model->GetScalarAndGradientOutputs( parameters, activeParameters,
                                              scalars, gradient );
  if ( error != madai::Model::NO_ERROR )
  {
    std::cerr << "Error encountered when computing scalar and gradient outputs."
              << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Model gradient for parameters [" << parameters[0] << ", "
            << parameters[1] << "]: [" << gradient[0] << ", " << gradient[1]
            << "]" << std::endl;

  // Clean up.
  delete model;

  return EXIT_SUCCESS;
}
