#include <cstdlib>

#include "Parameter.h"
#include "Trace.h"
#include "VectorUtilities.h"


int main( int argc, char* argv[] )
{
  // Create an example Trace
  madai::Trace trace;

  // Check that the size of the trace is 0 to begin with
  if ( trace.GetSize() != 0 ) {
    std::cerr << "Empty trace should have size 0, had size "
	      << trace.GetSize() << " instead." << std::endl;
    return EXIT_FAILURE;
  }

  // Test the Add() method that takes parameter values only. */
  std::vector< double > parameters;
  parameters.push_back( 1.0 );
  parameters.push_back( 2.3 );

  trace.Add( parameters );

  if ( trace.GetSize() != 1 ) {
    std::cerr << "Trace should have 1 element, has "
	      << trace.GetSize() << " instead." << std::endl;
    return EXIT_FAILURE;
  }

  madai::TraceElement element = trace[ 0 ];
  if ( element.m_ParameterValues != parameters ) {
    std::cerr << "Unexpected parameter values in first Add()" << std::endl;
    std::cerr << "Expected " << parameters << ", got "
	      << element.m_ParameterValues << " instead." << std::endl;
    return EXIT_FAILURE;
  }

  // Test the Add() method that takes parameter values and output values.
  std::vector< double > outputs;
  outputs.push_back( -2.1 );

  parameters[0] = -1.0;
  parameters[1] = 5.2;

  outputs[0] = -4.89;

  trace.Add( parameters, outputs );

  if ( trace.GetSize() != 2 ) {
    std::cerr << "Trace should have 2 elements, has "
	      << trace.GetSize() << " instead." << std::endl;
    return EXIT_FAILURE;
  }

  element = trace[ 1 ];

  if ( element.m_ParameterValues != parameters ) {
    std::cerr << "Unexpected parameter values in second Add()" << std::endl;
    std::cerr << "Expected " << parameters << ", got "
	      << element.m_ParameterValues << " instead." << std::endl;
    return EXIT_FAILURE;
  }

  if ( element.m_OutputValues != outputs ) {
    std::cerr << "Unexpected outputs in second Add()" << std::endl;
    std::cerr << "Expected " << outputs << ", got "
	      << element.m_OutputValues << std::endl;
    return EXIT_FAILURE;
  }

  // Test the Add() method that takes parameter values, output values,
  // and a log likelihood
  parameters[0] = -10.0;
  parameters[1] = 4;

  outputs[0] = -22.0;

  double logLikelihood = 0.82;
  trace.Add( parameters, outputs, logLikelihood );

  if ( trace.GetSize() != 3 ) {
    std::cerr << "Trace should have 3 elements, has "
	      << trace.GetSize() << " instead." << std::endl;
    return EXIT_FAILURE;
  }

  element = trace[ 2 ];

  if ( element.m_ParameterValues != parameters ) {
    std::cerr << "Unexpected parameter values in third Add()" << std::endl;
    std::cerr << "Expected " << parameters << ", got "
	      << element.m_ParameterValues << " instead." << std::endl;
    return EXIT_FAILURE;
  }

  if ( element.m_OutputValues != outputs ) {
    std::cerr << "Unexpected outputs in third Add()" << std::endl;
    std::cerr << "Expected " << outputs << ", got "
	      << element.m_OutputValues << std::endl;
    return EXIT_FAILURE;
  }

  if ( element.m_LogLikelihood != logLikelihood ) {
    std::cerr << "Unexpected log likelihood in thrid Add()" << std::endl;
    std::cerr << "Expected " << logLikelihood << ", got "
	      << element.m_LogLikelihood << std::endl;
    return EXIT_FAILURE;
  }


  // Write trace file
  std::vector< madai::Parameter > parameterVector;
  parameterVector.reserve( parameters.size() );
  for ( size_t i = 0; i < parameters.size(); ++i ) {
    parameterVector.push_back( madai::Parameter( "Parameter" ) );
  }

  std::vector< std::string > outputNames;
  outputNames.reserve( outputs.size() );
  for ( size_t i = 0; i < outputs.size(); ++i ) {
    outputNames.push_back( std::string( "Output" ) );
  }

  trace.WriteCSVFile( "TraceTest.csv", parameterVector, outputNames );

  return EXIT_SUCCESS;
}
