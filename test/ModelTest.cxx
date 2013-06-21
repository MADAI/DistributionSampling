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

#include "Model.h"

/** \class Test class for Model.
 *
 * Model is an abstract class, so we implement the pure virtual
 * methods in a derived class. */
class TestModel : public madai::Model {
public:
  TestModel()
  {
    this->AddParameter( "A" );             // Range is -DBL_MAX, DBL_MAX
    this->AddParameter( "BB", 0.0 );       // Rand is 0, DBL_MAX
    this->AddParameter( "CCC", 0.0, 1.0 ); // Range is 0, 1
  }

  virtual ~TestModel() {}

  virtual Model::ErrorType GetScalarOutputs( const std::vector< double > &,
                                             std::vector< double > & ) const
  {
    return Model::NO_ERROR;
  }

}; // end TestModel


int main( int, char *[] )
{

  TestModel * model = new TestModel();

  if ( model->IsReady() ) {
    std::cerr << "Model reports it is ready when it should not" << std::endl;
    return EXIT_FAILURE;
  }

  if ( model->GetNumberOfParameters() != 3 ) {
    std::cerr << "Model should have 3 parameters, has "
              << model->GetNumberOfParameters() << " instead." << std::endl;
    return EXIT_FAILURE;
  }

  std::vector< madai::Parameter > parameters = model->GetParameters();
  if ( parameters.size() != 3 ) {
    std::cerr << "Parameter vector should have 3 elements, has "
              << parameters.size() << " instead." << std::endl;
    return EXIT_FAILURE;
  }

  if ( parameters[0].m_Name != "A" ) {
    std::cerr << "Parameter 0 name should be 'A', was '"
              << parameters[0].m_Name << "' instead." << std::endl;
    return EXIT_FAILURE;
  }

  if ( parameters[1].m_Name != "BB" ) {
    std::cerr << "Parameter 1 name should be 'BB', was '"
              << parameters[1].m_Name << "' instead." << std::endl;
    return EXIT_FAILURE;
  }

  if ( parameters[2].m_Name != "CCC" ) {
    std::cerr << "Parameter 2 name should be 'BBB', was '"
              << parameters[2].m_Name << "' instead." << std::endl;
    return EXIT_FAILURE;
  }

  if ( model->GetNumberOfScalarOutputs() != 0 ) {
    std::cerr << "Number of scalar outputs should be 0 by default, was "
              << model->GetNumberOfScalarOutputs() << std::endl;
    return EXIT_FAILURE;
  }

  if ( model->GetScalarOutputNames().size() != 0 ) {
    std::cerr << "Size of vector containing scalar output names should be 0, was "
             << model->GetScalarOutputNames().size() << std::endl;
    return EXIT_FAILURE;
  }

  double h = 1.13;
  model->SetGradientEstimateStepSize( h );
  double retrievedH = model->GetGradientEstimateStepSize();
  if ( retrievedH != h ) {
    std::cerr << "Unexpected value from Model::GetGradientEstimateStepSize()" << std::endl;
    std::cerr << "Got " << retrievedH << " but expected " << h << std::endl;
    return EXIT_FAILURE;
  }

  if ( madai::Model::GetErrorTypeAsString( madai::Model::NO_ERROR ) != "NO_ERROR" ) {
    std::cerr << "Expected string 'NO_ERROR', got '"
              << madai::Model::GetErrorTypeAsString( madai::Model::NO_ERROR )
              << "' instead." << std::endl;
    return EXIT_FAILURE;
  }

  if ( madai::Model::GetErrorTypeAsString( madai::Model::INVALID_PARAMETER_INDEX ) != "INVALID_PARAMETER_INDEX" ) {
    std::cerr << "Expected string 'INVALID_PARAMETER_INDEX', got '"
              << madai::Model::GetErrorTypeAsString( madai::Model::INVALID_PARAMETER_INDEX )
              << "' instead." << std::endl;
    return EXIT_FAILURE;
  }

  if ( madai::Model::GetErrorTypeAsString( madai::Model::INVALID_ACTIVE_PARAMETERS ) != "INVALID_ACTIVE_PARAMETERS" ) {
    std::cerr << "Expected string 'INVALID_ACTIVE_PARAMETERS', got '"
              << madai::Model::GetErrorTypeAsString( madai::Model::INVALID_ACTIVE_PARAMETERS )
              << "' instead." << std::endl;
    return EXIT_FAILURE;
  }

  if ( madai::Model::GetErrorTypeAsString( madai::Model::FILE_NOT_FOUND_ERROR ) != "FILE_NOT_FOUND_ERROR" ) {
    std::cerr << "Expected string 'FILE_NOT_FOUND_ERROR', got '"
              << madai::Model::GetErrorTypeAsString( madai::Model::FILE_NOT_FOUND_ERROR )
              << "' instead." << std::endl;
    return EXIT_FAILURE;
  }

  if ( madai::Model::GetErrorTypeAsString( madai::Model::METHOD_NOT_IMPLEMENTED ) != "METHOD_NOT_IMPLEMENTED" ) {
    std::cerr << "Expected string 'METHOD_NOT_IMPLEMENTED', got '"
              << madai::Model::GetErrorTypeAsString( madai::Model::METHOD_NOT_IMPLEMENTED )
              << "' instead." << std::endl;
    return EXIT_FAILURE;
  }

  if ( madai::Model::GetErrorTypeAsString( madai::Model::OTHER_ERROR ) != "OTHER_ERROR" ) {
    std::cerr << "Expected string 'OTHER_ERROR', got '"
              << madai::Model::GetErrorTypeAsString( madai::Model::OTHER_ERROR )
              << "' instead." << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "ModelTest passed" << std::endl;

  return EXIT_SUCCESS;
}
