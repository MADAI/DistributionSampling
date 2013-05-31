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

#include "GaussianProcessEmulatorDirectoryReader.h"

#include "GaussianDistribution.h"
#include "GaussianProcessEmulator.h"
#include "Paths.h"
#include "RuntimeParameterFileReader.h"
#include "System.h"
#include "UniformDistribution.h"

#include <madaisys/Directory.hxx>
#include <madaisys/SystemTools.hxx>

#include <algorithm>  // std::find
#include <fstream>
#include <iostream> // std::cerr
#include <set>

#include <boost/algorithm/string.hpp>

using madaisys::SystemTools;


namespace madai {

GaussianProcessEmulatorDirectoryReader
::GaussianProcessEmulatorDirectoryReader() :
  m_Verbose( false )
{
}


GaussianProcessEmulatorDirectoryReader
::~GaussianProcessEmulatorDirectoryReader()
{
}


void
GaussianProcessEmulatorDirectoryReader
::SetVerbose( bool value )
{
  m_Verbose = value;
}


bool
GaussianProcessEmulatorDirectoryReader
::GetVerbose() const
{
  return m_Verbose;
}


namespace {


std::vector< std::string > getLineAsTokens( std::ifstream & is,
                                            std::string & line )
{
  std::vector< std::string > tokens;

  std::getline( is, line );

  // First, split on comment character
  std::vector< std::string > contentAndComments;
  boost::split( contentAndComments, line, boost::is_any_of("#") );

  if ( contentAndComments.size() == 0 ) {
    return tokens;
  }

  // Trim any leading or trailing spaces in the content
  boost::trim( contentAndComments[0] );

  // Next, split only the non-comment content
  boost::split( tokens, contentAndComments[0], boost::is_any_of("\t "),
                boost::token_compress_on );

  // If first token is empty string, remove it and return
  if ( tokens[0] == "" ) {
    tokens.erase( tokens.begin() );
    return tokens;
  }

  return tokens;
}

inline void discardWhitespace(std::istream & input)
{
  while (std::isspace(input.peek())) {
    input.get();
  }
}

/**
   Read a word from input.  If it equals the expected value, return
   true.  Else print error message to std::cerr and return false*/
inline bool CheckWord(std::istream & input, const std::string & expected)
{
  std::string s;
  if (! input.good()) {
    std::cerr << "premature end of input.  (" << expected << ")\n";
    return false;
  }
  input >> s;
  if (s == expected)
    return true;
  std::cerr
    << "format error.  Expected \"" << expected
    << "', but got '" << s << "'.\n";
  return false;
}

/**
   Read an integer from input.  If it equals the expected value, return
   true.  Else print error message to std::cerr and return false*/
inline bool CheckInteger(std::istream & input, int i,
    const std::string & errorMessage)
{
  if (! input.good()) {
    std::cerr << "premature end of input.  ("
      << errorMessage << ")\n";
    return false;
  }
  int j;
  input >> j;
  if (i == j)
    return true;
  std::cerr
    << "format error.  Expected '" << i
    << "', but got '" << j << "'.  ("
    << errorMessage << ")\n";
  return false;
}


bool parseInteger( int & x, std::istream & input )
{
  if (! input.good()) return false;
  input >> x;
  return true;
}

inline bool getIndex(
    const std::vector< std::string > & sv,
    const std::string & s,
    int & index)
{
  std::vector< std::string >::const_iterator x = std::find(sv.begin(), sv.end(), s);
  if (x == sv.end()) {
    index = -1;
    return false;
  } else {
    index = x - sv.begin();
    assert( static_cast< size_t >( index ) < sv.size() );
    return true;
  }
}

/**
   Read the parameter_priors.dat file in statistical_analysis. */
bool parseExperimentalResults(
    GaussianProcessEmulator & gpe,
    const std::string & experimentalResultsFileName,
    bool verbose )
{
  const std::vector< std::string > & outputNames = gpe.m_OutputNames;
  int numberOutputs = gpe.m_NumberOutputs;
  if ( static_cast< size_t >( numberOutputs ) != outputNames.size()) {
    std::cerr << "numberOutputs != outputNames.size()";
    return false;
  }

  if ( verbose ) {
    std::cout << "Opening experimental results file '"
              << experimentalResultsFileName << "'\n";
  }

  if ( !System::IsFile( experimentalResultsFileName ) ) {
    std::cerr << "Expected '" << experimentalResultsFileName
              << "' to be a file, but it does not exist or is a directory.\n";
    return false;
  }

  std::ifstream resultsFile( experimentalResultsFileName.c_str() );
  if ( !resultsFile.good() ) {
    std::cerr << "Error opening " << experimentalResultsFileName << '\n';
    return false;
  }

  // default values.
  gpe.m_ObservedValues = Eigen::VectorXd::Zero(numberOutputs);
  gpe.m_ObservedVariances = Eigen::VectorXd::Zero(numberOutputs);

  while ( resultsFile.good() ) {
    std::string line;
    std::vector< std::string > tokens = getLineAsTokens( resultsFile, line );

    // Skip empty lines
    if ( tokens.size() == 0 ) {
      continue;
    }

    std::string formatMessage( "Format should be <name> <value> <uncertainty>" );
    if ( tokens.size() < 3 ) {
      std::cerr << "Too few tokens in line '" << line << "' in file '"
                << experimentalResultsFileName << "'\n";
      std::cerr << formatMessage << "\n";
      resultsFile.close();
      return false;
    }

    if ( tokens.size() > 3 && verbose ) {
      std::cout << "Extra tokens in line '" << line << "' in file '"
                << experimentalResultsFileName << "'\n";
      std::cout << formatMessage << "\n";
    }

    std::string name( tokens[0] );
    double value = atof( tokens[1].c_str() );
    double uncertainty = atof( tokens[2].c_str() );

    int index = -1;
    bool success = getIndex( outputNames, name, index );
    if ( success ) {
      if ( verbose ) {
        std::cout << "Parsed output '" << name << "' with value " << value
                  << "\n";
      }

      gpe.m_ObservedValues(index) = value;
      gpe.m_ObservedVariances(index) = uncertainty;
    } else if ( verbose ) {
      std::cout << "Ignoring value and uncertainty for unknown output name '" << name << "'\n";
    }
  }

  resultsFile.close();

  return true;
}


/**
   Read the parameter_priors.dat file in statistical_analysis. */
bool parseParameters(
    std::vector< madai::Parameter > & parameters,
    int & numberParameters,
    const std::string & statisticalAnalysisDirectory,
    bool verbose )
{
  // First check to see if file exists
  std::string priorFileName = statisticalAnalysisDirectory + Paths::SEPARATOR +
    Paths::PARAMETER_PRIORS_FILE;

  if ( verbose ) {
    std::cout << "Opened parameter priors file '" << priorFileName << "'.\n";
  }

  if ( !System::IsFile( priorFileName ) ) {
    std::cerr << "Expected '" << priorFileName
              << "' to be a file, but it does not exist or is a directory.\n";
    return false;
  }

  std::ifstream input( priorFileName.c_str() );
  if ( !input.good() ) {
    std::cerr << "Could not read parameter prior file '" <<
      priorFileName << "\n";
    return false;
  }

  parameters.clear(); // Empty the output vector

  while ( input.good() ) {
    std::string line;
    std::vector< std::string > tokens = getLineAsTokens( input, line );

    // Skip empty or comment lines
    if ( tokens.size() == 0 ) continue;

    // Check that we have four tokens (distribution type, name,
    // distribution parameters)
    std::string formatMessage
      ( "Format should be <distribution type> <name> "
        "<distribution parameter 1> <distribution parameter 2>" );
    if ( tokens.size() < 4 ) {
      std::cerr << "Too few tokens in line '" << line << "' in file '"
                << priorFileName << "'\n";
      std::cerr << formatMessage << "\n";
      input.close();
      return false;
    }

    if ( tokens.size() > 4 && verbose ) {
      std::cout << "Extra tokens in line '" << line << "' in file '"
                << priorFileName << "'\n";
      std::cout << formatMessage << "\n";
    }

    std::string type( tokens[0] );
    std::string name( tokens[1] );
    boost::algorithm::to_lower( type );

    // All supported distributions have two double parameters
    double distributionValues[2];
    distributionValues[0] = atof( tokens[2].c_str() );
    distributionValues[1] = atof( tokens[3].c_str() );

    if ( type == "uniform" ) {

      madai::UniformDistribution priorDist;
      priorDist.SetMinimum( distributionValues[0] );
      priorDist.SetMaximum( distributionValues[1] );
      parameters.push_back( madai::Parameter( name, priorDist ) );

      if ( verbose ) {
        std::cout << "Parsed '" << type << "'-distributed parameter '"
                  << name << " with minimum " << distributionValues[0]
                  << " and maximum " << distributionValues[1] << std::endl;
      }

    } else if ( type == "gaussian" ) {

      madai::GaussianDistribution priorDist;
      priorDist.SetMean( distributionValues[0] );
      priorDist.SetStandardDeviation( distributionValues[1] );
      parameters.push_back( madai::Parameter( name, priorDist ) );

      if ( verbose ) {
        std::cout << "Parsed '" << type << "'-distributed parameter '"
                  << name << " with mean " << distributionValues[0]
                  << " and standard deviation " << distributionValues[1]
                  << std::endl;
      }

    } else {

      std::cerr << "Expected uniform or gaussian, but got " <<
        type << "\n";
      input.close();
      return false;

    }
  }

  input.close();

  numberParameters = parameters.size();

  return ( numberParameters > 0 );
}


/**
   Read the CovarianceFunctionType from the command line. */
bool parseCovarianceFunction(
    GaussianProcessEmulator::CovarianceFunctionType & covarianceType,
    std::istream & input)
{
  if (! input.good()) return false;
  std::string s;
  input >> s;
  if (s == "POWER_EXPONENTIAL_FUNCTION")
    covarianceType = GaussianProcessEmulator::POWER_EXPONENTIAL_FUNCTION;
  else if (s == "SQUARE_EXPONENTIAL_FUNCTION")
    covarianceType = GaussianProcessEmulator::SQUARE_EXPONENTIAL_FUNCTION;
  else if (s == "MATERN_32_FUNCTION")
    covarianceType = GaussianProcessEmulator::MATERN_32_FUNCTION;
  else if (s == "MATERN_52_FUNCTION")
    covarianceType = GaussianProcessEmulator::MATERN_52_FUNCTION;
  else {
    return false;
  }
  return true;
}


bool parseOutputs(
    std::vector< std::string > & outputNames,
    int & numberOutputs,
    const std::string & statisticalAnalysisDirectory,
    bool verbose)
{
  // First check to see if file exists
  std::string observablesFileName = statisticalAnalysisDirectory +
    Paths::SEPARATOR + Paths::OBSERVABLE_NAMES_FILE;

  if ( !System::IsFile( observablesFileName ) ) {
    std::cerr << "Expected '" << observablesFileName
              << "' to be a file, but it does not exist or is a directory.\n";
    return false;
  }

  std::ifstream input( observablesFileName.c_str() );
  if ( !input.good() ) {
    std::cerr << "Could not read observables file '"
              << observablesFileName << "'\n";
    return false;
  }

  if ( verbose ) {
    std::cout << "Opened file '" << observablesFileName << "'.\n";
  }

  outputNames.clear(); // Empty the output vector

  while ( input.good() ) {
    std::string line;
    std::vector< std::string > tokens = getLineAsTokens( input, line );

    // Skip lines with no tokens
    if ( tokens.size() == 0 ) continue;

    // Warn if there is more than one token on a line
    if ( tokens.size() > 1 && verbose ) {
      std::cout << "Extra tokens in line '" << line << "' in file "
                << observablesFileName << "'\n";
      std::cout << "Format should be <observable name>\n";
    }

    outputNames.push_back( tokens[0] );

    if ( verbose ) {
      std::cout << "Parsed output '" << tokens[0] << "'.\n";
    }

  }

  input.close();

  numberOutputs = outputNames.size();

  return (numberOutputs > 0);
}


bool parseNumberOfModelRuns( int & x,
                             const std::string & modelOutputDirectory )
{
  madaisys::Directory directory;
  if ( !directory.Load( modelOutputDirectory.c_str() ) ) {
    std::cerr << "Couldn't read directory '" << modelOutputDirectory << "'\n";
    return false;
  }

  unsigned int runCounter = 0;

  for ( unsigned long i = 0; i < directory.GetNumberOfFiles(); ++i ) {

    // \todo Verify that the file staring with "run" is a directory
    std::string directoryName( directory.GetFile( i ) );
    if ( directoryName.substr( 0, 3 ) == "run" ) {
      runCounter++;
    }
  }

  x = runCounter;
  assert ( x > 0 );

  return true;
}


template < typename TDerived >
inline bool parseParameterAndOutputValues(
    Eigen::MatrixBase< TDerived > & parameterValues_,
    Eigen::MatrixBase< TDerived > & outputValues_,
    Eigen::MatrixBase< TDerived > & outputUncertainty_,
    const std::string & modelOutputDirectory,
    unsigned int numberTrainingPoints,
    std::vector< madai::Parameter > parameters,
    std::vector< std::string > outputNames,
    bool verbose )
{
  // Get the list of directories in model_outputs/
  madaisys::Directory directory;
  if ( !directory.Load( modelOutputDirectory.c_str() ) ) {
    return false;
  }

  std::vector< std::string > runDirectories;
  for ( unsigned long i = 0; i < directory.GetNumberOfFiles(); ++i ) {
    std::string fileName( directory.GetFile( i ) );
    std::string filePath( modelOutputDirectory + fileName );
    if ( fileName.find_first_of( "run" ) == 0 ) {
      runDirectories.push_back( fileName );
    }
  }
  std::sort( runDirectories.begin(), runDirectories.end() );

  size_t numberParameters = parameters.size();
  size_t numberOutputs = outputNames.size();

  // Copy parameterValues_
  Eigen::MatrixBase< TDerived > & parameterValues
    = const_cast< Eigen::MatrixBase< TDerived > & >( parameterValues_ );
  // Copy outputValues_
  Eigen::MatrixBase< TDerived > & outputValues
    = const_cast< Eigen::MatrixBase< TDerived > & >( outputValues_ );
  // Copy outputUncertainty_
  Eigen::MatrixBase< TDerived > & outputUncertainty
    = const_cast< Eigen::MatrixBase< TDerived > & >( outputUncertainty_ );

  parameterValues.derived().resize( numberTrainingPoints, numberParameters );
  outputValues.derived().resize( numberTrainingPoints, numberOutputs );
  outputUncertainty.derived().resize( numberOutputs, 1 );

  unsigned int runCounter = 0;
  std::vector< double > averageUncertainty( numberOutputs, 0.0 );

  for ( size_t i = 0; i < runDirectories.size(); ++i ) {
    std::string directoryName( runDirectories[i] );
    if ( verbose )
      std::cout << "Run directory name: '" << directoryName << "'\n";

    if ( directoryName.substr( 0, 3 ) == "run" ) {
      // Open the parameters file
      std::string parametersFileName = modelOutputDirectory +
        Paths::SEPARATOR + directoryName + Paths::SEPARATOR +
        Paths::PARAMETERS_FILE;

      if ( !System::IsFile( parametersFileName ) ) {
        std::cerr << "Expected '" << parametersFileName
                  << "' to be a file, but it does not exist or is a directory.\n";
        return false;
      }

      std::ifstream parametersFile( parametersFileName.c_str() );
      if ( !parametersFile.good() ) {
        std::cerr << "Could not read parameter file '" << parametersFileName
                  << "'\n";
        return false;
      }
      if ( verbose ) {
        std::cout << "Opened file '" << parametersFileName << "'\n";
      }

      // Keep track of which parameters were read
      std::set< std::string > parametersRemaining;
      for ( size_t p = 0; p < parameters.size(); ++p ) {
        parametersRemaining.insert( parameters[p].m_Name );
      }

      while ( parametersFile.good() ) {
        std::string line;
        std::vector< std::string > tokens = getLineAsTokens( parametersFile,
                                                             line );

        // Skip lines with no tokens
        if ( tokens.size() == 0 ) continue;

        std::string formatString( "Format should be <parameter name> "
                                  "<parameter value>" );
        if ( tokens.size() < 2 ) {
          std::cerr << "Too few tokens in line '" << line << "' in file '"
                    << parametersFileName << "'\n";
          std::cerr << formatString << "\n";
          parametersFile.close();
          return false;
        }

        if ( tokens.size() > 2 && verbose ) {
          std::cout << "Extra tokens in line '" << line << "' in file '"
                    << parametersFileName << "' will be ignored.\n";
          std::cout << formatString << "\n";
        }

        std::string name( tokens[0] );
        double value = atof( tokens[1].c_str() );

        // Find the index of the parameter
        bool foundParameter = false;
        for ( unsigned int i = 0; i < parameters.size(); i++ ) {
          if ( name == parameters[i].m_Name ) {
            parameterValues( runCounter, i) = value;
            foundParameter = true;
            parametersRemaining.erase( name );
            if ( verbose )
              std::cout << "Parsed parameter '" << name << "' with value "
                        << parameterValues( runCounter, i ) << std::endl;
            break;
          }
        }

        if ( !foundParameter && verbose ) {
          std::cout << "Unknown parameter name '" << name << "' in line '"
                    << line << "' in file '" << parametersFileName
                    << "' will be ignored.\n";
        }

      }
      parametersFile.close();

      // Check that we have read values for all parameters
      if ( parametersRemaining.size() > 0 ) {
        std::cerr << "Values were not read for all parameters in file '"
                  << parametersFileName << "'\n";
        std::cerr << "Missing values for:\n";
        std::set< std::string >::iterator iter;
        for ( iter = parametersRemaining.begin();
              iter != parametersRemaining.end();
              ++iter ) {
          std::cerr << "  " << *iter << "\n";
        }
        return false;
      }

      // Open the results file
      std::string resultsFileName = modelOutputDirectory +
        Paths::SEPARATOR + directoryName + Paths::SEPARATOR +
        Paths::RESULTS_FILE;

      if ( !System::IsFile( resultsFileName ) ) {
        std::cerr << "Expected '" << resultsFileName
                  << "' to be a file, but it does not exist or is a directory.\n";
        return false;
      }

      std::ifstream resultsFile ( resultsFileName.c_str() );
      if ( !resultsFile.good() ) {
        std::cerr << "Could not read results file '" << resultsFileName
                  << "'\n";
        return false;
      }
      if ( verbose )
        std::cout << "Opened file " << resultsFileName << "'\n";

      // Keep track of which parameters were read
      std::set< std::string > outputsRemaining;
      for ( size_t p = 0; p < outputNames.size(); ++p ) {
        outputsRemaining.insert( outputNames[p] );
      }

      while ( resultsFile.good() ) {
        std::string line;
        std::vector< std::string > tokens = getLineAsTokens( resultsFile,
                                                             line );

        // Skip lines with no tokens
        if ( tokens.size() == 0 ) continue;

        std::string formatString( "Format should be <output name> "
                                  "<output value> [output uncertainty]\n" );

        // Check for two few tokens
        if ( tokens.size() < 2 ) {
          std::cerr << "Too few tokens in line '" << line << "' in file '"
                    << resultsFileName << "'\n";
          std::cerr << formatString << "\n";
          resultsFile.close();
          return false;
        }

        // Too many tokens
        if ( tokens.size() > 4 && verbose ) {
          std::cout << "Extra tokens in line '" << line << "' in file '"
                    << resultsFileName << "' will be ignored.\n";
          std::cout << formatString << "\n";
        }

        std::string name( tokens[0] );

        // Find the output index
        int outputIndex = -1;
        if ( getIndex( outputNames, name, outputIndex ) ) {
          double value = atof( tokens[1].c_str() );
          outputValues( runCounter, outputIndex ) = value;

          double uncertainty = 0.0;
          if ( tokens.size() == 3 ) {
            uncertainty = atof( tokens[2].c_str() );
          }
          averageUncertainty[ outputIndex ] += uncertainty;

          outputsRemaining.erase( name );
        } else if ( verbose ) {
          std::cout << "Unknown output name '" << name << "' in line '"
                    << line << "' in file '" << resultsFileName
                    << "' will be ignored.\n";
        }

      }

      resultsFile.close();

      // Check that we have read values for all parameters
      if ( outputsRemaining.size() > 0 ) {
        std::cerr << "Values were not read for all results in file '"
                  << resultsFileName << "'\n";
        std::cerr << "Missing values for:\n";
        std::set< std::string >::iterator iter;
        for ( iter = outputsRemaining.begin();
              iter != outputsRemaining.end();
              ++iter ) {
          std::cerr << "  " << *iter << "\n";
        }
        return false;
      }


      runCounter++;
    }
  }

  // Average the uncertainty across model runs
  for ( unsigned int i = 0; i < numberOutputs; i++ ) {
    outputUncertainty( i, 0 ) = averageUncertainty[i] / double( runCounter );
  }

  return true;
}


bool parseModelDataDirectoryStructure(
    GaussianProcessEmulator & gpme,
    const std::string & modelOutputDirectory,
    const std::string & statisticalAnalysisDirectory,
    bool verbose )
{
  if ( !parseParameters(
          gpme.m_Parameters,
          gpme.m_NumberParameters,
          statisticalAnalysisDirectory,
          verbose ) ) {
    std::cerr << "Couldn't parse parameters\n";
    return false;
  }
  if ( !parseOutputs(
          gpme.m_OutputNames,
          gpme.m_NumberOutputs,
          statisticalAnalysisDirectory,
          verbose) ) {
    std::cerr << "Couldn't parse outputs\n";
    return false;
  }

  // Initialize means and uncertainty scales
  gpme.m_TrainingOutputMeans = Eigen::VectorXd::Constant( gpme.m_NumberOutputs, 0.0 );
  gpme.m_TrainingOutputVarianceMeans = Eigen::VectorXd::Constant( gpme.m_NumberOutputs, 0.0 );

  if ( !parseNumberOfModelRuns( gpme.m_NumberTrainingPoints,
                                modelOutputDirectory ) ) {
    std::cerr << "Couldn't parse number of model runs.\n";
    return false;
  }
  Eigen::MatrixXd TMat( gpme.m_NumberOutputs, 1 );
  if ( !parseParameterAndOutputValues(
          gpme.m_TrainingParameterValues, gpme.m_TrainingOutputValues, TMat,
          modelOutputDirectory, gpme.m_NumberTrainingPoints,
          gpme.m_Parameters, gpme.m_OutputNames, verbose ) ) {
    std::cerr << "parse Parameter and Output Values error\n";
    return false;
  }
  gpme.m_TrainingOutputVarianceMeans = TMat.col(0);

  return true;
}


/**
   Read and save comments to a vector of strings.  Will append
   comments to existing comments. */
bool parseComments(
    std::vector< std::string > & returnStrings,
    std::istream & i)
{
  static const char comment_character = '#';
  if (! i.good()) return false;
  int c = i.peek();
  while ( i.good() && ( ( c == comment_character ) || ( c == '\n') ) ) {
    std::string s;
    std::getline(i, s);
    returnStrings.push_back(s);
    c = i.peek();
  }
  if (i.fail()) return false;
  return true;
}


/**
   Populates a matrix from a istream.  Expects positive integers
   number_rows and number_columns to be listed first. Reads elements
   in row-major order.

   Returns false if reading from the istream fails.  Otherwise
   true. */
template < typename TDerived >
inline bool ReadMatrix(
    const Eigen::MatrixBase< TDerived > & m_,
    std::istream & ins)
{
  unsigned int nrows, ncols;
  Eigen::MatrixBase< TDerived > & m
    = const_cast< Eigen::MatrixBase< TDerived > & >(m_);
  if (! ins.good()) return false;
  ins >> nrows >> ncols;
  m.derived().resize(nrows, ncols);
  for (size_t i = 0; i < nrows; ++i)
    for (size_t j = 0; j < ncols; ++j) {
      if (! ins.good()) return false;
      ins >> m(i,j);
    }
  return true;
}


/**
   Populates a Vector from a istream.  Expects positive integers
   number_elements to be listed first. Reads elements in order.

   Returns false if reading from the istream fails.  Otherwise true.
 */
template < typename TDerived >
inline bool ReadVector(
    const Eigen::MatrixBase< TDerived > & v_,
    std::istream & ins)
{
  unsigned int nrows;
  Eigen::MatrixBase< TDerived > & v
    = const_cast< Eigen::MatrixBase< TDerived > & >(v_);
  if (! ins.good()) return false;
  ins >> nrows;
  v.derived().resize(nrows, 1);
  for (size_t i = 0; i < nrows; ++i) {
    if (! ins.good()) return false;
    ins >> v(i,0);
  }
  return true;
}

bool parseSubmodels(
    GaussianProcessEmulator::SingleModel & m,
    int modelIndex,
    std::istream & input) {
  std::string word;
  if (! CheckWord(input, "MODEL")) return false;
  if (! CheckInteger(input, modelIndex, "modelIndex")) return false;
  while (input.good()) {
    if (! input.good()) return false;
    input >> word;
    if (word == "COVARIANCE_FUNCTION") {
      if (!parseCovarianceFunction(m.m_CovarianceFunction,  input)) {
        std::cerr << "Could not parse COVARIANCE_FUNCTION when reading "
                  << "submodel " << modelIndex << "\n";
        return false;
      }
    } else if (word == "REGRESSION_ORDER") {
      if (! parseInteger(m.m_RegressionOrder, input)) {
        std::cerr << "Could not parse REGRESSION_ORDER when reading "
                  << "submodel " << modelIndex << "\n";
        return false;
      }
    } else if (word == "THETAS") {
      if (! ReadVector(m.m_Thetas, input)) {
        std::cerr << "Could not parse THETAS when reading "
                  << "submodel " << modelIndex << "\n";
        return false;
      }
    } else if (word == "END_OF_MODEL") {
      return true;
    } else {
      std::cerr << "Unexpected keyword: '" << word << "' when reading "
                << "submodel.\n";
      return false;
    }
  }
  return false;
}


bool parsePCADecomposition(
    GaussianProcessEmulator & gpme,
    std::istream & input ) {
  parseComments(gpme.m_Comments,input);

  bool outputMeansRead             = false;
  bool outputUncertaintyScalesRead = false;
  bool outputPCAEigenvaluesRead    = false;
  bool outputPCAEigenvectorsRead   = false;

  std::string word;
  while (input.good()) {
    if (! input.good()) return false;
    input >> word;
    if (word == "OUTPUT_MEANS") {
      if (! ReadVector(gpme.m_TrainingOutputMeans, input)) {
        std::cerr << "Could not read OUTPUT_MEANS entry in PCA decomposition "
                  << "file.\n";
        return false;
      }
      outputMeansRead = true;
    } else if (word == "OUTPUT_UNCERTAINTY_SCALES") {
      if (! ReadVector(gpme.m_UncertaintyScales, input)) {
        std::cerr << "Could not read OUTPUT_UNCERTAINTY_SCALES in PCA "
                  << "decomposition file.\n";
        return false;
      }
      outputUncertaintyScalesRead = true;
    } else if (word == "OUTPUT_PCA_EIGENVALUES") {
      if (! ReadVector(gpme.m_PCAEigenvalues, input)) {
        std::cerr << "Could not read OUTPUT_PCA_EIGENVALUES in PCA "
                  << "decomposition file.\n";
        return false;
      }
      outputPCAEigenvaluesRead = true;
    } else if (word == "OUTPUT_PCA_EIGENVECTORS") {
      if (! ReadMatrix(gpme.m_PCAEigenvectors, input)) {
        std::cerr << "Could not read OUTPUT_PCA_EIGENVECTORS in PCA "
                  << "decomposition file.\n";
        return false;
      }
      outputPCAEigenvectorsRead = true;
    } else if (word == "END_OF_FILE") {
      break;
    } else {
      std::cerr << "Unexpected keyword: '" << word << "' in PCA "
                << "decomposition file.\n";
      return false;
    }
  }

  if ( !( outputMeansRead && outputUncertaintyScalesRead &&
          outputPCAEigenvaluesRead && outputPCAEigenvectorsRead ) ) {
    std::cerr << "Not all required PCA components read.\n";
    std::cerr << "Missing:\n";
    if ( !outputMeansRead )
      std::cerr << "OUTPUT_MEANS\n";
    if ( !outputUncertaintyScalesRead )
      std::cerr << "OUTPUT_UNCERTAINTY_SCALES\n";
    if ( !outputPCAEigenvaluesRead )
      std::cerr << "OUTPUT_PCA_EIGENVALUES\n";
    if ( !outputPCAEigenvectorsRead )
      std::cerr << "OUTPUT_PCA_EIGENVECTORS\n";

    return false;
  }

  return true;
}


bool parseGaussianProcessEmulator(
    GaussianProcessEmulator & gpme,
    const std::string & statisticalAnalysisDirectory) {
  std::string emulatorFile = statisticalAnalysisDirectory + Paths::SEPARATOR +
    Paths::EMULATOR_STATE_FILE;

  if ( !System::IsFile( emulatorFile ) ) {
    std::cerr << "Expected '" << emulatorFile
              << "' to be a file, but it does not exist or is a directory.\n";
    return false;
  }

  std::ifstream input( emulatorFile.c_str() );
  parseComments(gpme.m_Comments,input);
  std::string word;
  while (input.good()) {
    if (! input.good()) return false;
    input >> word;
    if (word == "SUBMODELS") {
      if (! parseInteger(gpme.m_NumberPCAOutputs, input)) {
        std::cerr << "Could not parse number of SUBMODELS when reading "
                  << "emulator state.\n";
        input.close();
        return false;
      }
      gpme.m_PCADecomposedModels.resize(gpme.m_NumberPCAOutputs);
      for (int i = 0; i < gpme.m_NumberPCAOutputs; ++i) {
        if (! parseSubmodels(gpme.m_PCADecomposedModels[i],i,input)) {
          std::cerr << "Could not parse submodels when reading emulator "
                    << "state.\n";
          input.close();
          return false;
        }
        gpme.m_PCADecomposedModels[i].m_Parent = &gpme;
      }
    } else if (word == "END_OF_FILE") {
      return true;
    } else {
      std::cerr << "Unexpected keyword: " << word << "\n";
      input.close();
      return false;
    }
  }

  input.close();

  return true;
}


} // end anonymous namespace


/**
  This takes an empty GPEM and loads training data.
  \returns true on success. */
bool
GaussianProcessEmulatorDirectoryReader
::LoadTrainingData(GaussianProcessEmulator * gpe,
                    std::string modelOutputDirectory,
                    std::string statisticalAnalysisDirectory,
                    std::string experimentalResultsFileName)
{
  if ( !parseModelDataDirectoryStructure(*gpe,
                                         modelOutputDirectory,
                                         statisticalAnalysisDirectory,
                                         m_Verbose ) )
    return false;

  if (! parseExperimentalResults( *gpe, experimentalResultsFileName,
                                  this->m_Verbose )) {
    std::cerr << "Error in parseExperimentalResults()\n";
    return false;
  }

  return (gpe->CheckStatus() == GaussianProcessEmulator::UNTRAINED);
}


/**
  This takes a GPEM and loads PCA data.
  \returns true on success. */
bool
GaussianProcessEmulatorDirectoryReader
::LoadPCA(GaussianProcessEmulator * gpe,
          const std::string & statisticsDirectory)
{
  std::string PCAFile = statisticsDirectory + Paths::SEPARATOR +
    Paths::PCA_DECOMPOSITION_FILE;

  if ( !System::IsFile( PCAFile ) ) {
    std::cerr << "Expected '" << PCAFile
              << "' to be a file, but it does not exist or is a directory.\n";
    return false;
  }

  std::ifstream input( PCAFile.c_str() );
  if ( !parsePCADecomposition(*gpe, input) ) {
    std::cerr << "Error parsing PCA data.\n";
    input.close();
    return false;
  }

  // Initialize the retained principal components
  std::string runtimeParameterFile = statisticsDirectory + Paths::SEPARATOR +
    Paths::RUNTIME_PARAMETER_FILE;
  RuntimeParameterFileReader runtimeParameterReader;
  if ( !runtimeParameterReader.ParseFile( runtimeParameterFile ) ) {
    std::cerr << "Error parsing runtime parameters.\n" << std::endl;
  }

  double DEFAULT_PCA_FRACTION_RESOLVING_POWER = 0.95;
  // \todo should use madai::Defaults::PCA_FRACTION_RESOLVING_POWER
  double fractionalResolvingPower = runtimeParameterReader.GetOptionAsDouble(
      "PCA_FRACTION_RESOLVING_POWER",
      DEFAULT_PCA_FRACTION_RESOLVING_POWER);

  gpe->RetainPrincipalComponents( fractionalResolvingPower );

  // We are finished reading the input files.
  return (gpe->CheckStatus() == GaussianProcessEmulator::UNTRAINED);
}


/**
  This takes a GPEM and loads the emulator specific
  data (submodels with their thetas).
  \returns true on success. */
bool
GaussianProcessEmulatorDirectoryReader
::LoadEmulator(GaussianProcessEmulator * gpe,
               const std::string & statisticalAnalysisDirectory)
{
  if ( !parseGaussianProcessEmulator(*gpe, statisticalAnalysisDirectory) ) {
    std::cerr << "Error parsing gaussian process emulator.\n";
    return false;
  }
  // We are finished reading the input files.
  if ( gpe->CheckStatus() != GaussianProcessEmulator::UNCACHED ) {
    std::cerr << "Emulator already cached.\n";
    std::cerr << gpe->GetStatusAsString() << "\n";
    return false;
  }
  if ( !gpe->MakeCache() ) {
    std::cerr << "Error while making cache.\n";
    return false;
  }
  return true;
}

} // end namespace madai
