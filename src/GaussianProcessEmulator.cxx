/*=========================================================================
 *
 *  Copyright 2011-2013 The University of North Carolina at Chapel Hill
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

/*
 * GaussianProcessEmulator Class
 *
 * \author Hal Canary <cs.unc.edu/~hal/>
 * \author Christopher Coleman-Smith <cec24@phy.duke.edu>
 * \author Cory Quammen <cs.unc.edu/~cquammen>
 */

#include <cmath>        // std::exp std::amp
#include <limits>       // std::numeric_limits
#include <fstream>      // std::ofstream std::ifstream
#include "GaussianProcessEmulator.h"
#include "UniformDistribution.h"
#include "GaussianDistribution.h"
namespace {
using namespace madai;

/**
   The standard clamp macro. */
template <typename T>
inline T clamp(const T & x, const T & low, const T & high) {
  return (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)));
}

/**
   Read a word from input.  If it equals the expected value, return
   true.  Else print error message to std::cerr and return false*/
inline bool CheckWord(std::istream & input, const std::string & expected) {
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
    << "\", but got \"" << s << "\".\n";
  return false;
}

/**
   Read an integer from input.  If it equals the expected value, return
   true.  Else print error message to std::cerr and return false*/
inline bool CheckInteger(std::istream & input, int i,
    const std::string & errorMessage) {
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
    << "format error.  Expected \"" << i
    << "\", but got \"" << j << "\".  ("
    << errorMessage << ")\n";
  return false;
}


/**
   Read and save comments to a vector of strings.  Will append
   comments to existing comments. */
bool parseComments(
    std::vector< std::string > & returnStrings,
    std::istream & i) {
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


std::ostream & serializeComments(
    const std::vector< std::string > & comments,
    std::ostream & o) {
  for (unsigned int i = 0; i < comments.size(); ++i) {
    o << comments[i] << '\n';
  }
  return o;
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

/**
   Print a Matrix to output stream, preceded by its dimensions.  Use
   row-major order. */
template < typename TDerived >
static inline std::ostream & PrintMatrix(
    const Eigen::MatrixBase< TDerived > & m,
    std::ostream & o)
{
  o << m.rows() << ' ' << m.cols() << '\n';
  if (m.cols() > 0) {
    for (int i = 0; i < m.rows(); ++i) {
      for (int j = 0; j < (m.cols() - 1); ++j)
        o << m(i, j) << '\t';
      o << m(i, m.cols() - 1) << '\n';
    }
  }
  return o;
}

/**
   Print a Vector to output stream, preceded by its size. */
template < typename TDerived >
static inline std::ostream & PrintVector(
    const Eigen::MatrixBase< TDerived > & v,
    std::ostream & o)
{
  o << v.size() << '\n';
  for (int i = 0; i < v.size(); ++i) {
    o << v(i) << '\n';
  }
  return o;
}


/**
 * A covariance function can be represented as a string. */
const char * GetCovarianceFunctionString(
  GaussianProcessEmulator::CovarianceFunctionType cov)
{
  switch (cov) {
  case GaussianProcessEmulator::POWER_EXPONENTIAL_FUNCTION:  return "POWER_EXPONENTIAL_FUNCTION";
  case GaussianProcessEmulator::SQUARE_EXPONENTIAL_FUNCTION: return "SQUARE_EXPONENTIAL_FUNCTION";
  case GaussianProcessEmulator::MATERN_32_FUNCTION:  return "MATERN_32_FUNCTION";
  case GaussianProcessEmulator::MATERN_52_FUNCTION:  return "MATERN_52_FUNCTION";
  default:
    assert(false);
    return "UNKNOWN";
  }
}
/**
   Read the CovarianceFunctionType from the command line. */
bool parseCovarianceFunction(
    GaussianProcessEmulator::CovarianceFunctionType & cov,
    std::istream & input) {
  if (! input.good()) return false;
  std::string s;
  input >> s;
  if (s == "POWER_EXPONENTIAL_FUNCTION")
    cov = GaussianProcessEmulator::POWER_EXPONENTIAL_FUNCTION;
  else if (s == "SQUARE_EXPONENTIAL_FUNCTION")
    cov = GaussianProcessEmulator::SQUARE_EXPONENTIAL_FUNCTION;
  else if (s == "MATERN_32_FUNCTION")
    cov = GaussianProcessEmulator::MATERN_32_FUNCTION;
  else if (s == "MATERN_52_FUNCTION")
    cov = GaussianProcessEmulator::MATERN_52_FUNCTION;
  else {
    return false;
  }
  return true;
}

/**
   Read the parameter_priors.dat file in statistical_analysis. */
bool parseParameters(
    std::vector< madai::Parameter > & parameters,
    int & numberParameters,
    std::string AnalysisDir ) {
  // First check to see if file exists
  std::string PriorFileName = AnalysisDir+"/parameter_priors.dat";
  std::ifstream input( PriorFileName.c_str() );
  if ( !input.good() ) return false;
  parameters.clear(); // Empty the output vector
  while ( input.good() ) {
    while ( input.peek() == '#' ) { // Disregard commented lines
      std::string s;
      std::getline( input, s );
    }
    std::string name;
    std::string type;
    double dist_vals[2];
    if ( !(input >> name >> type >> dist_vals[0] >> dist_vals[1]) )
      break;
    if ( type == "uniform" ) {
      madai::UniformDistribution priorDist;
      priorDist.SetMinimum(dist_vals[0]);
      priorDist.SetMaximum(dist_vals[1]);
      parameters.push_back(madai::Parameter( name, priorDist ) );
    } else if ( type == "gaussian" ) {
      madai::GaussianDistribution priorDist;
      priorDist.SetMean(dist_vals[0]);
      priorDist.SetStandardDeviation(dist_vals[1]);
      parameters.push_back(madai::Parameter( name, priorDist ) );
    } else {
      std::cerr << "Expected uniform or gaussian, but got " <<
        type << "\n";
      return false;
    }
  }
  numberParameters = parameters.size();
  assert( numberParameters > 0 );
  
  return true;
}

/**
   Read an integer from the input, followed by that number of
   parameters.  Populate the numberParameters vector. */
bool parseParameters(
    std::vector<madai::Parameter> & parameters,
    int & numberParameters,
    std::istream & input) {
  if (! input.good()) return false;
  input >> numberParameters;
  parameters.reserve(numberParameters);
  for(int i = 0; i < numberParameters; ++i) {
    std::string name, priotDistType;
    if (! input.good()) return false;
    input >> name >> priotDistType;
    if (priotDistType == "UNIFORM") {
      double min, max;
      input >> min >> max;
      madai::UniformDistribution priorDist;
      priorDist.SetMinimum(min);
      priorDist.SetMaximum(max);
      parameters.push_back(madai::Parameter(name, priorDist));
    } else if (priotDistType == "GAUSSIAN") {
      double mean, std;
      input >> mean >> std;
      madai::GaussianDistribution priorDist;
      priorDist.SetMean(mean);
      priorDist.SetStandardDeviation(std);
      parameters.push_back(madai::Parameter(name, priorDist));
    } else {
      std::cerr << "Expected UNIFORM or GAUSSIAN, but got " <<
        priotDistType <<'\n';
      return false;
    }
  } // end for loop
  return true;
}

std::ostream & serializeParameters(
    const std::vector<madai::Parameter> & parameters,
    std::ostream & o) {
  int numberParameters = parameters.size();
  o << numberParameters << '\n';
  for(int i = 0; i < numberParameters; ++i) {
    o << parameters[i].m_Name << '\t';
    const Distribution * priorDist = parameters[i].GetPriorDistribution();
    const UniformDistribution * uniformPriorDist
      = dynamic_cast<const UniformDistribution *>(priorDist);
    const GaussianDistribution * gaussianPriorDist
      = dynamic_cast<const GaussianDistribution *>(priorDist);
    if (uniformPriorDist != NULL) {
      o << "UNIFORM" << '\t'
        << uniformPriorDist->GetMinimum() << '\t'
        << uniformPriorDist->GetMaximum() << '\n';
    } else if (gaussianPriorDist != NULL) {
      o << "GAUSSIAN" << '\t'
        << gaussianPriorDist->GetMean() << '\t'
        << gaussianPriorDist->GetStandardDeviation() << '\n';
    } else {
      assert(false);
      o << "UNKNOWN_PRIOR_TYPE\t0\t1\n";
    }
  }
  return o;
}


bool parseOutputs(
    std::vector< std::string > & outputNames,
    int & numberOutputs,
    std::string AnalysisDir ) {
  // First cehck to see if file exists
  std::string ObservablesFileName = AnalysisDir+"/observable_names.dat";
  std::ifstream input ( ObservablesFileName.c_str() );
  if ( !input.good() ) return false;
  outputNames.clear(); // Empty the output vector
  while ( !input.eof() ) {
    while ( input.peek() == '#' ) { // Disregard commented lines
      std::string s;
      std::getline( input, s );
    }
    std::string name;
    input >> name;
    outputNames.push_back( name );
  }
  if ( outputNames.back() == "" || outputNames.back() == " " ) {
    outputNames.pop_back();
  }
  numberOutputs = outputNames.size();
  assert( numberOutputs > 0 );
  
  return true;
}

bool parseOutputs(
    std::vector< std::string > & outputNames,
    int & numberOutputs,
    std::istream & input) {
  if (! input.good()) return false;
  input >> numberOutputs;
  outputNames.resize(numberOutputs);
  for(int i = 0; i < numberOutputs; ++i) {
    if (! input.good()) return false;
    input >> outputNames[i];
  }
  return true;
}

std::ostream & serializeStringVector(
    const std::vector< std::string > & strings,
    std::ostream & o) {
  o << strings.size() << '\n';
  for(unsigned int i = 0; i < strings.size(); ++i) {
    o << strings[i] << '\n';
  }
  return o;
}

bool parseInteger( int & x, std::istream & input ) {
  if (! input.good()) return false;
  input >> x;
  return true;
}

bool parseNumberOfModelRuns( int & x, std::string ModelOutDir ) {
  std::string command = "ls -1 "+ModelOutDir+" > dirlist";
  std::system( command.c_str() );
  
  std::string dirlist = "dirlist";
  std::ifstream DFile ( dirlist.c_str() );
  if ( !DFile.good() ) return false;
  unsigned int run_counter = 0;
  while ( !DFile.eof() ) {
    std::string dir_name;
    DFile >> dir_name;
    char* temp = new char[3]();
    std::strncpy( temp, dir_name.c_str(), 3 );
    if ( std::strcmp( temp, "run" ) == 0 ) {
      run_counter++;
    }
  }
  std::remove( dirlist.c_str() );
  x = run_counter;
  assert ( x > 0 );
  
  return true;
}

template < typename TDerived >
inline bool parseParameterAndOutputValues(
    const Eigen::MatrixBase< TDerived > & m_,
    const Eigen::MatrixBase< TDerived > & m2_,
    std::string ModelOutDir,
    unsigned int numberTrainingPoints,
    std::vector< madai::Parameter > parameters,
    std::vector< std::string > outputNames ) {
  // Get the list of directories in model_outputs/
  std::string command = "ls -1 "+ModelOutDir+" > dirlist";
  std::system( command.c_str() );
  std::string dirlist = "dirlist";
  std::ifstream DFile ( dirlist.c_str() );
  int p = parameters.size();
  int t = outputNames.size();
  // Copy m_
  Eigen::MatrixBase< TDerived > & m
  = const_cast< Eigen::MatrixBase< TDerived > & >(m_);
  // Copy m2_
  Eigen::MatrixBase< TDerived > & m2
  = const_cast< Eigen::MatrixBase< TDerived > & >(m2_);
  m.derived().resize( numberTrainingPoints, p );
  m2.derived().resize( numberTrainingPoints, t );
  unsigned int run_counter = 0;
  if ( !DFile.good() ) return false;
  while ( !DFile.eof() ) {
    std::string dir_name;
    DFile >> dir_name;
    char* temp = new char[3]();
    std::strncpy( temp, dir_name.c_str(), 3 );
    if ( std::strcmp( temp, "run" ) == 0 ) {
      // Open the parameters.dat file
      std::string par_file_name = ModelOutDir+dir_name+"/parameters.dat";
      std::ifstream parfile ( par_file_name.c_str() );
      if ( !parfile.good() ) return false;
      while ( !parfile.eof() ) {
        while ( parfile.peek() == '#' ) {
          std::string tline;
          std::getline( parfile, tline );
        }
        std::string name;
        parfile >> name;
        for ( unsigned int i = 0; i < p; i++ )
          if ( name == parameters[i].m_Name )
            parfile >> m( run_counter, i);
      }
      parfile.close();
      // Open the results.dat file
      std::string results_file_name = ModelOutDir+dir_name+"/results.dat";
      std::ifstream results_file ( results_file_name.c_str() );
      // Check the style of the outputs
      std::string line;
      while ( results_file.peek() == '#' ) // Disregard comments go to first output
        std::getline( results_file, line );
      char* temp1 = new char[100]();
      std::getline( results_file, line );
      std::strncpy( temp1, line.c_str(), 100 );
      char* token = strtok( temp1, " " );
      int NVal = 0;
      while ( true ) {
        NVal++;
        token = strtok( NULL, " " );
        if ( token == NULL )
          break;
      }
      results_file.seekg( 0, results_file.beg);
      if ( !results_file.good() ) return false;
      while ( !results_file.eof() ) {
        while ( results_file.peek() == '#' ) // Disregard comments, go to next output
          std::getline( results_file, line );
        std::string name;
        results_file >> name;
        for ( unsigned int i = 0; i < t; i++ )
          if ( name == outputNames[i] ) {
            results_file >> m2( run_counter, i );
            if ( NVal == 3 ) {
              double ModelUnc;
              results_file >> ModelUnc;
            } else if ( NVal != 2 ) {
              std::cerr << "Unknown output format error.\n";
              return false;
            }
          }
      }
      results_file.close();
      run_counter++;
    }
  }
  std::remove( dirlist.c_str() );
  
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
        std::cerr << "parse error\n"; // \todo error message
        return false;
      }
    } else if (word == "Z_VALUES") {
      if (! ReadVector(m.m_ZValues, input)) {
        std::cerr << "parse error\n"; // \todo error message
        return false;
      }
    } else if (word == "REGRESSION_ORDER") {
      if (! parseInteger(m.m_RegressionOrder, input)) {
        std::cerr << "parse error\n"; // \todo error message
        return false;
      }
    } else if (word == "THETAS") {
      if (! ReadVector(m.m_Thetas, input)) {
        std::cerr << "parse error\n"; // \todo error message
        return false;
      }
    } else if (word == "END_OF_MODEL") {
      return true;
    } else {
      std::cerr << "Unexected keyword: \"" << word << "\"\n";
      return false;
    }
  }
  return false;
}

std::ostream & serializeSubmodels(
    const GaussianProcessEmulator::SingleModel & m,
    int modelIndex,
    std::ostream & o) {
  o << "MODEL " << modelIndex << '\n';
  o << "COVARIANCE_FUNCTION\t"
    << GetCovarianceFunctionString(m.m_CovarianceFunction) << '\n';
  o << "REGRESSION_ORDER\t" << m.m_RegressionOrder << '\n';
  o << "Z_VALUES\n";
  PrintVector(m.m_ZValues, o);
  o << "THETAS\n";
  PrintVector(m.m_Thetas, o);
  o << "END_OF_MODEL\n";
  return o;
}

std::ostream & serializeGaussianProcessEmulator(
    const GaussianProcessEmulator & gpme,
    std::ostream & o) {

  serializeComments(gpme.m_Comments,o);
  o << "VERSION 1\n";
  o << "PARAMETERS\n";
  serializeParameters(gpme.m_Parameters,o);
  o << "OUTPUTS\n";
  serializeStringVector(gpme.m_OutputNames,o);
  o << "NUMBER_OF_TRAINING_POINTS\t"
    << gpme.m_NumberTrainingPoints << '\n';
  o << "PARAMETER_VALUES\n";
  PrintMatrix(gpme.m_ParameterValues, o);
  o << "OUTPUT_VALUES\n";
  PrintMatrix(gpme.m_OutputValues, o);
  o << "OUTPUT_MEANS\n";
  PrintVector(gpme.m_OutputMeans, o);
  o << "OUTPUT_UNCERTAINTY_SCALES\n";
  PrintVector(gpme.m_OutputUncertaintyScales, o);
  o << "OUTPUT_PCA_EIGENVALUES\n";
  PrintVector(gpme.m_PCAEigenvalues, o);
  o << "OUTPUT_PCA_EIGENVECTORS\n";
  PrintMatrix(gpme.m_PCAEigenvectors, o);
  o << "SUBMODELS\t"
    << gpme.m_NumberPCAOutputs << "\n";
  for (int i = 0; i < gpme.m_NumberPCAOutputs; ++i) {
    serializeSubmodels(gpme.m_PCADecomposedModels[i],i,o);
  }
  o << "END_OF_FILE\n";
  return o;
}

std::ostream & serializePCADecomposition(
    const GaussianProcessEmulator & gpme,
    std::ostream & o ) {
  serializeComments(gpme.m_Comments,o);
  o << "VERSION 1\n";
  o << "OUTPUT_MEANS\n";
  PrintVector(gpme.m_OutputMeans, o);
  o << "OUTPUT_UNCERTAINTY_SCALES\n";
  PrintVector(gpme.m_OutputUncertaintyScales, o);
  o << "OUTPUT_PCA_EIGENVALUES\n";
  PrintVector(gpme.m_PCAEigenvalues, o);
  o << "OUTPUT_PCA_EIGENVECTORS\n";
  PrintMatrix(gpme.m_PCAEigenvectors, o);
  o << "SUBMODELS\t" << gpme.m_NumberPCAOutputs << "\n";
  for ( unsigned int i = 0; i < gpme.m_NumberPCAOutputs; ++i ) {
    serializeSubmodels(gpme.m_PCADecomposedModels[i],i,o);
  }
  o << "END_OF_FILE\n";
  return o;
}
  
bool parseModelDataDirectoryStructure(
    GaussianProcessEmulator & gpme,
    std::string Model_Outs_Dir,
    std::string Stat_Anal_Dir) {
  if ( !parseParameters( 
          gpme.m_Parameters, gpme.m_NumberParameters, Stat_Anal_Dir ) ) {
    std::cerr << "parse Parameters error\n";
    return false;
  }
  if ( !parseOutputs( 
          gpme.m_OutputNames, gpme.m_NumberOutputs, Stat_Anal_Dir ) ) {
    std::cerr << "parse Outputs error\n";
    return false;
  }
  if ( !parseNumberOfModelRuns( 
          gpme.m_NumberTrainingPoints, Model_Outs_Dir ) ) {
    std::cerr << "parse Integer error\n";
    return false;
  }
  if ( !parseParameterAndOutputValues( 
          gpme.m_ParameterValues, gpme.m_OutputValues, Model_Outs_Dir, 
          gpme.m_NumberTrainingPoints, gpme.m_Parameters, gpme.m_OutputNames ) ) {
    std::cerr << "parse Parameter and Output Values error\n";
    return false;
  }
  return true;
}

bool parsePCADecomposition(
    GaussianProcessEmulator & gpme,
    std::istream & input ) {
  parseComments(gpme.m_Comments,input);
  std::string word;
  if (! CheckWord(input, "VERSION")) return false;
  if (! CheckInteger(input, 1, "versn")) return false;
  while (input.good()) {
    if (! input.good()) return false;
    input >> word;
    if (word == "OUTPUT_MEANS") {
      if (! ReadVector(gpme.m_OutputMeans, input)) {
        std::cerr << "parse error\n"; // \todo error message
        return false;
      }
    } else if (word == "OUTPUT_UNCERTAINTY_SCALES") {
      if (! ReadVector(gpme.m_OutputUncertaintyScales, input)) {
        std::cerr << "parse error\n"; // \todo error message
        return false;
      }
    } else if (word == "OUTPUT_PCA_EIGENVALUES") {
      if (! ReadVector(gpme.m_PCAEigenvalues, input)) {
        std::cerr << "parse error\n"; // \todo error message
        return false;
      }
    } else if (word == "OUTPUT_PCA_EIGENVECTORS") {
      if (! ReadMatrix(gpme.m_PCAEigenvectors, input)) {
        std::cerr << "parse error\n"; // \todo error message
        return false;
      }
    } else if (word == "SUBMODELS") {
      if (! parseInteger(gpme.m_NumberPCAOutputs, input)) {
        std::cerr << "parse error\n"; // \todo error message
        return false;
      }
      gpme.m_PCADecomposedModels.resize(gpme.m_NumberPCAOutputs);
      for (int i = 0; i < gpme.m_NumberPCAOutputs; ++i) {
        if (! parseSubmodels(gpme.m_PCADecomposedModels[i],i,input)) {
          std::cerr << "parse error\n"; // \todo error message
          return false;
        }
        gpme.m_PCADecomposedModels[i].m_Parent = &gpme;
      }
    } else if (word == "END_OF_FILE") {
      return true;
    } else {
      std::cerr << "Unexected keyword: \"" << word << "\"\n";
      return false;
    }
  }
  return false;
}


bool parseGaussianProcessEmulator(
    GaussianProcessEmulator & gpme,
    std::string TopDirectory,
    bool LoadPCA ) {
  std::string MOD = TopDirectory+"/model_output/";
  std::string SAD = TopDirectory+"/statistical_analysis/";
  if ( !parseModelDataDirectoryStructure( gpme, MOD, SAD ) ) {
    std::cerr << "parse directory structure error\n";
    return false;
  }
  std::string PCADecompFileName = SAD+"/PCADecomposition.dat";
  if ( LoadPCA ) {
    std::ifstream input( PCADecompFileName.c_str() );
    if ( input ) {
      // File exists, read in from it
      if ( !parsePCADecomposition( gpme, input ) ) {
        std::cerr << "parse saved PCA Decomposition error\n";
        return false;
      }
    } else {
      std::cerr << "No saved PCA Decomposition.\n";
      return false;
    }
  }
  return true;
}

bool parseGaussianProcessEmulator(
    GaussianProcessEmulator & gpme,
    std::istream & input) {
  parseComments(gpme.m_Comments,input);
  std::string word;
  if (! CheckWord(input, "VERSION")) return false;
  if (! CheckInteger(input, 1, "versn")) return false;
  while (input.good()) {
    if (! input.good()) return false;
    input >> word;
    if (word == "PARAMETERS") {
      if (! parseParameters(
              gpme.m_Parameters, gpme.m_NumberParameters, input)) {
        std::cerr << "parse error\n"; // \todo error message
        return false;
      }
    } else if (word == "OUTPUTS") {
      if (! parseOutputs(
              gpme.m_OutputNames, gpme.m_NumberOutputs, input)) {
        std::cerr << "parse error\n"; // \todo error message
        return false;
      }
    } else if (word == "NUMBER_OF_TRAINING_POINTS") {
      if (! parseInteger(gpme.m_NumberTrainingPoints, input)) {
        std::cerr << "parse error\n"; // \todo error message
        return false;
      }
    } else if (word == "PARAMETER_VALUES") {
      if (! ReadMatrix(gpme.m_ParameterValues, input)) {
        std::cerr << "parse error\n"; // \todo error message
        return false;
      }
    } else if (word == "OUTPUT_VALUES") {
      if (! ReadMatrix(gpme.m_OutputValues, input)) {
        std::cerr << "parse error\n"; // \todo error message
        return false;
      }
    } else if (word == "OUTPUT_MEANS") {
      if (! ReadVector(gpme.m_OutputMeans, input)) {
        std::cerr << "parse error\n"; // \todo error message
        return false;
      }
    } else if (word == "OUTPUT_UNCERTAINTY_SCALES") {
      if (! ReadVector(gpme.m_OutputUncertaintyScales, input)) {
        std::cerr << "parse error\n"; // \todo error message
        return false;
      }
    } else if (word == "OUTPUT_PCA_EIGENVALUES") {
      if (! ReadVector(gpme.m_PCAEigenvalues, input)) {
        std::cerr << "parse error\n"; // \todo error message
        return false;
      }
    } else if (word == "OUTPUT_PCA_EIGENVECTORS") {
      if (! ReadMatrix(gpme.m_PCAEigenvectors, input)) {
        std::cerr << "parse error\n"; // \todo error message
        return false;
      }
    } else if (word == "SUBMODELS") {
      if (! parseInteger(gpme.m_NumberPCAOutputs, input)) {
        std::cerr << "parse error\n"; // \todo error message
        return false;
      }
      gpme.m_PCADecomposedModels.resize(gpme.m_NumberPCAOutputs);
      for (int i = 0; i < gpme.m_NumberPCAOutputs; ++i) {
        if (! parseSubmodels(gpme.m_PCADecomposedModels[i],i,input)) {
        std::cerr << "parse error\n"; // \todo error message
        return false;
        }
        gpme.m_PCADecomposedModels[i].m_Parent = &gpme;
      }
    } else if (word == "END_OF_FILE") {
      return true;
    } else {
      std::cerr << "Unexected keyword: \"" << word << "\"\n";
      return false;
    }
  }
  return false;
}

inline int NumberRegressionFunctions(
    int regressionOrder,
    int numberParameters) {
  return 1 + (regressionOrder * numberParameters);
}

inline int NumberThetas(
    GaussianProcessEmulator::CovarianceFunctionType cf,
    int numberParameters) {
  switch(cf) {
  case GaussianProcessEmulator::SQUARE_EXPONENTIAL_FUNCTION:
    return numberParameters + 2;
  case GaussianProcessEmulator::POWER_EXPONENTIAL_FUNCTION:
    return numberParameters + 3;
  case GaussianProcessEmulator::MATERN_32_FUNCTION:
    return 3;
  case GaussianProcessEmulator::MATERN_52_FUNCTION:
    return 3;
  case GaussianProcessEmulator::UNKNOWN_FUNCTION:
    //fall through
  default:
    return -1;
  }
}

const char * stat(GaussianProcessEmulator::StatusType s) {
  switch(s) {
  case GaussianProcessEmulator::READY        : return "READY";
  case GaussianProcessEmulator::UNCACHED     : return "UNCACHED";
  case GaussianProcessEmulator::UNTRAINED    : return "UNTRAINED";
  case GaussianProcessEmulator::UNINITIALIZED: return "UNINITIALIZED";
  case GaussianProcessEmulator::ERROR        :
  default                                    : return "ERROR";
  }
}

template < typename TDerived >
inline void MakeHMatrix(
    const Eigen::MatrixBase< TDerived > & X,
    const Eigen::MatrixBase< TDerived > & H_,
    int regressionOrder)
{
  int p = X.cols(), N = X.rows();
  int numberRegressionFunctions = 1 + (regressionOrder * p);
  Eigen::MatrixBase< TDerived > & H
    = const_cast< Eigen::MatrixBase< TDerived > & >(H_);
  H.derived().resize(N, numberRegressionFunctions);
  H.block(0,0,N,1) = Eigen::MatrixXd::Constant(N,1, 1.0);
  if (regressionOrder > 0)
    H.block(0,1,N,p) = X;
  for (int i = 1; i < regressionOrder; ++i) {
    H.block(0,1+(i*p),N,p) = H.block(0,1+((i-1)*p),N,p).cwiseProduct(X);
  }
}
template < typename TDerived >
inline void MakeHVector(
    const Eigen::MatrixBase< TDerived > & point,
    const Eigen::MatrixBase< TDerived > & hvec_,
    int regressionOrder)
{
  int p = point.size();
  int numberRegressionFunctions = 1 + (regressionOrder * p);
  Eigen::MatrixBase< TDerived > & hvec
    = const_cast< Eigen::MatrixBase< TDerived > & >(hvec_);
  hvec.derived().resize(numberRegressionFunctions,1);
  hvec(0) = 1.0;
  if (regressionOrder > 0)
    hvec.segment(1,p) = point;
  for (int i = 1; i < regressionOrder; ++i) {
    hvec.segment(1+(i*p),p)
      = hvec.segment(1+((i-1)*p),p).cwiseProduct(point);
  }
}

} // anonymous namespace

namespace madai {

double GaussianProcessEmulator::SingleModel::CovarianceCalc(
    const Eigen::VectorXd & v1, const Eigen::VectorXd & v2) const
{
  int p = m_Parent->m_NumberParameters;
  int numberThetas = m_Thetas.size();
  assert(numberThetas > 0);
  switch(m_CovarianceFunction) {
  case POWER_EXPONENTIAL_FUNCTION:
    {
      assert(numberThetas == (p + 3));
      assert ((m_Thetas(2) > 0.0) && (m_Thetas(2) <= 2.0));
      // theta(0) == ln(amplitude)
      // theta(1) == nugget
      // theta(2) == power
      double covariance = 0.0;
      double exponent = 0.0;
      for(int i = 0; i < p; i++){
        double d = std::abs(v1(i) - v2(i));
        exponent += (-0.5) * std::pow(
            d / std::exp(std::abs(m_Thetas(i + 3))), m_Thetas(2));
      }
      covariance = exp(m_Thetas(0) + exponent);
      if (exponent < 1e-5)
        covariance += exp(m_Thetas(1));
      return covariance;
    }
  case SQUARE_EXPONENTIAL_FUNCTION:
    {
      assert(numberThetas == (p + 2));
      int truecount  = 0;
      double covariance = 0.0;
      double exponent = 0.0;
      // theta(0) == ln(amplitude)
      // theta(1) == nugget
      // theta(2+i) == scale[i]
      for(int i = 0; i < p; i++){
        double d = std::abs(v1(i) - v2(i));
        exponent += (-0.5) * std::pow(d / std::exp(m_Thetas(i + 2)), 2);
        if (d < 1e-10)
          truecount++;
      }
      covariance = exp(m_Thetas(0)) * exp(exponent);
      if(truecount == p) {
        covariance += exp(m_Thetas(1));
      }
      return covariance;
    }
  case MATERN_32_FUNCTION:
    {
      assert(numberThetas == 3);
      static const double ROOT3 = 1.7320508075688772;
      double distance = 0.0;
      double covariance;
      int truecount = 0;
      // map the m_Thetas onto some local variables so the formula
      // is more transparent
      double amp = m_Thetas(0);
      double nugget = m_Thetas(1);
      double rho = std::exp(m_Thetas(2));
      // calculate the euclidean distance between the two points;
      for(int i = 0; i < p; i++) {
        double d = std::pow(std::abs(v1(i) - v2(i)), 2);
        distance += d;
        if (d < 1e-16)
          ++truecount;
        // this is currently the distance squared
      }
      distance = std::sqrt(distance); // reduce back to the right dimensions
      if (distance > 0.0)
        covariance = amp * (1 + ROOT3 * (distance / rho))
          * std::exp(-ROOT3 * (distance / rho));
      else
        covariance = amp;
      if (truecount == p)
        covariance += nugget;
      return covariance;
    }
  case MATERN_52_FUNCTION:
    {
      assert(numberThetas == 3);
      int truecount = 0;
      double distance = 0.0;
      double covariance = 0.0;
      // map the m_Thetas onto some local variables so the formula
      // is more transparent
      double amp = m_Thetas(0);
      double nugget = m_Thetas(1);
      double rho = std::exp(m_Thetas(2));
      static const double ROOT5 = 2.23606797749979;
      double d_over_r = 0.0;
      // calculate the euclidean distance between the two points;
      for (int i = 0; i < p; i++){
        double d = std::abs(v1(i) - v2(i));
        // this is currently the distance squared
        distance += std::pow(d, 2.0);
        if (d < 1e-16)
          ++truecount;
      }
      // reduce back to the right dimensions
      distance = std::sqrt(distance);
      d_over_r = distance / rho;
      if (distance > 0.0){
        covariance
          = amp * (1 + ROOT5 * d_over_r + (5.0 / 3.0) * d_over_r * d_over_r)
          * std:: exp(- ROOT5 * d_over_r);
      } else {
        covariance = amp;
      }
      // this means we're on a diagonal term, golly but i write bad code :(
      if (truecount == p){
        covariance += nugget;
      }
      return covariance;
    }
  default:
    assert(false);
  }
  return 0.0;
}


bool GaussianProcessEmulator::Load(std::string TopDirectory) {
  m_Status = UNINITIALIZED;
  if ( !parseGaussianProcessEmulator(*this, TopDirectory, true) ) {
    std::cerr << "Error parsing gaussian process emulator.\n";
    return false;
  }
  // We are finished reading the input files.
  if ( this->CheckStatus() != GaussianProcessEmulator::UNCACHED ) {
    std::cerr << "Emulator already cached.\n";
    std::cerr << stat(this->CheckStatus()) << "\n";
    return false;
  }
  if ( !this->MakeCache() ) {
    std::cerr << "Error while makeing cache.\n";
    return false;
  }
  return true;
}


bool GaussianProcessEmulator::Load(std::istream & input)
{
  m_Status = UNINITIALIZED;
  if (! parseGaussianProcessEmulator(*this, input)) {
    std::cerr << "FIXME need error message\n";
    return false;
  }
  // We are finished reading the input file.
  if (this->CheckStatus() != GaussianProcessEmulator::UNCACHED) {
    std::cerr << "FIXME status error\t" << stat(this->CheckStatus())
    << '\n';
    return false;
  }
  if (! this->MakeCache()) {
    std::cerr << "FIXME need error message\n";
    return false;
  }
  return true;
}

GaussianProcessEmulator::StatusType
GaussianProcessEmulator::CheckStatus() {
  m_Status = UNINITIALIZED;
  if (m_NumberTrainingPoints < 1) {
    return m_Status;
  }
  if (m_NumberParameters < 1) {
    return m_Status;
  }
  if (m_NumberOutputs < 1) {
    return m_Status;
  }
  if (static_cast<int>(m_Parameters.size()) != m_NumberParameters) {
    return m_Status;
  }
  if (static_cast<int>(m_OutputNames.size()) != m_NumberOutputs) {
    return m_Status;
  }
  if (m_ParameterValues.rows() != m_NumberTrainingPoints) {
    return m_Status;
  }
  if (m_OutputValues.rows() != m_NumberTrainingPoints) {
    return m_Status;
  }
  if (m_ParameterValues.cols() != m_NumberParameters) {
    return m_Status;
  }
  if (m_OutputValues.cols() != m_NumberOutputs) {
    return m_Status;
  }
  if(m_OutputUncertaintyScales.size() != m_NumberOutputs) {
    m_OutputUncertaintyScales
      = Eigen::VectorXd::Constant(m_NumberOutputs,1.0);
  }
  m_Status = UNTRAINED;
  if(m_NumberPCAOutputs < 1)
    return m_Status;
  if(m_OutputMeans.size() != m_NumberOutputs)
    return m_Status;
  if(m_PCAEigenvalues.size() != m_NumberPCAOutputs)
    return m_Status;
  if(m_PCAEigenvectors.rows() != m_NumberOutputs)
    return m_Status;
  if(m_PCAEigenvectors.cols() != m_NumberPCAOutputs)
    return m_Status;
  if (static_cast<int>(m_PCADecomposedModels.size()) != m_NumberPCAOutputs)
    return m_Status;
  for (int i = 0; i < m_NumberPCAOutputs; ++i) {
    SingleModel & m = m_PCADecomposedModels[i];
    if (m.m_Parent == NULL)
      return m_Status;
    if (m.m_CovarianceFunction == UNKNOWN_FUNCTION)
      return m_Status;
    if (m.m_RegressionOrder < 0)
      return m_Status;
    if (m.m_ZValues.size() != m_NumberTrainingPoints)
      return m_Status;
    if (m.m_Thetas.size() !=
        NumberThetas(m.m_CovarianceFunction, m_NumberParameters))
      return m_Status;
  }
  m_Status = UNCACHED;
  for (int i = 0; i < m_NumberPCAOutputs; ++i) {
    SingleModel & m = m_PCADecomposedModels[i];
    int F = NumberRegressionFunctions(m.m_RegressionOrder,m_NumberParameters);
    if (m.m_CInverse.rows() != m_NumberTrainingPoints)
      return m_Status;
    if (m.m_CInverse.cols() != m_NumberTrainingPoints)
      return m_Status;
    if (m.m_RegressionMatrix1.rows() != F)
      return m_Status;
    if (m.m_RegressionMatrix1.cols() != F)
      return m_Status;
    if (m.m_RegressionMatrix2.rows() != F)
      return m_Status;
    if (m.m_RegressionMatrix2.cols() != m_NumberTrainingPoints)
      return m_Status;
    if (m.m_BetaVector.size() != F)
      return m_Status;
    if (m.m_GammaVector.size() != m_NumberTrainingPoints)
      return m_Status;
  }
  m_Status = READY;
  return m_Status;
}


/**
   Set default values to uninitialized values. */
GaussianProcessEmulator::SingleModel::SingleModel() :
  m_Parent(NULL),
  m_CovarianceFunction(UNKNOWN_FUNCTION),
  m_RegressionOrder(-1)
{ }


/**
   Once Load(), Train(), or BasicTraining() finishes, calculate and
   cache some data to make calling GetEmulatorOutputsAndCovariance()
   faster. */
bool GaussianProcessEmulator::MakeCache() {
  if ((m_Status != READY) && (m_Status != UNCACHED))
    return false;
  assert(m_NumberPCAOutputs == static_cast<int>(m_PCADecomposedModels.size()));
  for (int i = 0; i < m_NumberPCAOutputs; ++i) {
    if (! m_PCADecomposedModels[i].MakeCache())
      return false;
  }
  m_Status = READY;
  return true;
}

bool GaussianProcessEmulator::SingleModel::MakeCache() {
  int N = m_Parent->m_NumberTrainingPoints;
  int p = m_Parent->m_NumberParameters;
  int F = NumberRegressionFunctions(m_RegressionOrder, p);
  Eigen::MatrixXd & X = m_Parent->m_ParameterValues;

  // allocate members
  m_CInverse.resize(N, N);
  m_BetaVector.resize(F);
  m_GammaVector.resize(N);
  m_RegressionMatrix1.resize(N, F);
  m_RegressionMatrix2.resize(F, N);

  // local matrices that appear in formulae.
  Eigen::MatrixXd HMatrix(N, F);
  Eigen::MatrixXd CMatrix(N, N);

  // CALCULATE HMatrix
  MakeHMatrix(X, HMatrix, m_RegressionOrder);

  // CALCULATE CMatrix
  // CMatrix is the covariance matrix of the design with itself.
  for (int j = 0; j < N; ++j) {
    Eigen::VectorXd rowj = X.row(j);
    for (int k = j; k < N; ++k) {
      Eigen::VectorXd rowk = X.row(k);
      CMatrix(j,k) = this->CovarianceCalc(rowj, rowk);
      if (j != k)
        CMatrix(k,j) = CMatrix(j,k);
    }
  }

  // CALCULATE CACHE VARIABLES
  m_CInverse = CMatrix.ldlt().solve(Eigen::MatrixXd::Identity(N,N));

  m_RegressionMatrix1 = (HMatrix.transpose() * m_CInverse *
                        HMatrix).ldlt().solve(Eigen::MatrixXd::Identity(F,F));

  m_RegressionMatrix2 = (m_CInverse * HMatrix).transpose();

  m_BetaVector = (m_RegressionMatrix1 * HMatrix.transpose() *
                  m_CInverse * m_ZValues);
  m_GammaVector = m_CInverse * (m_ZValues - (HMatrix * m_BetaVector));
  return true;
}

/**
   Default to uninitialized state. */
GaussianProcessEmulator::GaussianProcessEmulator() :
  m_Status(UNINITIALIZED),
  m_NumberParameters(0),
  m_NumberOutputs(0),
  m_NumberTrainingPoints(0),
  m_NumberPCAOutputs(0)
{ }

/**
   This takes an empty GPEM and loads training data */
bool GaussianProcessEmulator::LoadTrainingData(std::istream & input) {
  m_Status = UNINITIALIZED;
  if (! parseGaussianProcessEmulator(*this, input))
    return false;
  m_NumberPCAOutputs = 0;
  this->CheckStatus();
  return (m_Status == UNTRAINED);
}

/**
   This taken an empty GPEM and loads training data */
bool GaussianProcessEmulator::LoadTrainingData(std::string TopDirectory) {
  m_Status = UNINITIALIZED;
  if ( !parseGaussianProcessEmulator(*this, TopDirectory, false) )
    return false;
  m_NumberPCAOutputs = 0;
  this->CheckStatus();
  return (m_Status == UNTRAINED);
}

/**
   This takes an GPEM and trains it. \returns true on sucess. */
bool GaussianProcessEmulator::SingleModel::Train(
    GaussianProcessEmulator::CovarianceFunctionType covarianceFunction,
    int regressionOrder)
{
  if (regressionOrder < 0) {
    /* \todo error message to stderr */
    return false;
  }
  if (regressionOrder > 3) {
    /* \todo error message to stderr */
    return false;
  }
  m_CovarianceFunction = covarianceFunction;
  m_RegressionOrder = regressionOrder;
  int numberThetas
    = NumberThetas(covarianceFunction, m_Parent->m_NumberParameters);
  m_Thetas.resize(numberThetas);
  std::cerr << "Sorry, but this function is not yet implemented.\n";
  // FIXME
  return false;
}


/**
   This takes an GPEM and trains it. \returns true on sucess. */
bool GaussianProcessEmulator::Train(
    GaussianProcessEmulator::CovarianceFunctionType covarianceFunction,
    int regressionOrder,
    double fractionResolvingPower)
{
  if (this->CheckStatus() == UNINITIALIZED)
    return false;
  m_Status = UNTRAINED;
  if (! this->PrincipalComponentDecompose(fractionResolvingPower))
    return false;
  for (int i = 0; i < m_NumberPCAOutputs; ++i) {
    if (! m_PCADecomposedModels[i].Train(covarianceFunction,regressionOrder))
      return false;
  }
  if (! this->MakeCache()) {
    std::cerr << "FIXME need error message\n";
    return false;
  }
  return true;
}

/**
   This takes an GPEM and trains it. \returns true on sucess. */
bool GaussianProcessEmulator::BasicTraining(
    double fractionResolvingPower,
    CovarianceFunctionType covarianceFunction,
    int regressionOrder,
    double defaultNugget,
    double amplitude,
    double scale)
{
  if (this->CheckStatus() == UNINITIALIZED)
    return false;
  m_Status = UNTRAINED;
  if (! this->PrincipalComponentDecompose(fractionResolvingPower))
    return false;
  for (int i = 0; i < m_NumberPCAOutputs; ++i) {
    if (! m_PCADecomposedModels[i].BasicTraining(covarianceFunction,
            regressionOrder, defaultNugget, amplitude, scale))
      return false;
  }
  m_Status = UNCACHED;
  if (! this->MakeCache()) {
    std::cerr << "FIXME need error message\n";
    return false;
  }
  return true;
}
/**
   Sets default values for all of the hyperparameters. \returns
   true on success. */
bool GaussianProcessEmulator::SingleModel::BasicTraining(
    CovarianceFunctionType covarianceFunction,
    int regressionOrder,
    double defaultNugget,
    double amplitude,
    double scale) {
  m_CovarianceFunction = covarianceFunction;
  m_RegressionOrder = regressionOrder;
  int p = m_Parent->m_NumberParameters;
  m_Thetas.resize( NumberThetas(m_CovarianceFunction, p));
  scale = std::abs(scale);
  switch(m_CovarianceFunction) {
  case GaussianProcessEmulator::SQUARE_EXPONENTIAL_FUNCTION:
    m_Thetas.resize(2 + p);
    m_Thetas(0) = amplitude;
    m_Thetas(1) = defaultNugget;
    for (int j = 0; j < p; ++j) {
      madai::Parameter & param = m_Parent->m_Parameters[j];
      madai::Distribution * priordist = param.m_PriorDistribution;
      m_Thetas(2+j) = scale * std::abs(priordist->GetPercentile(0.75)
                                     - priordist->GetPercentile(0.25));
    }
    break;
  case GaussianProcessEmulator::POWER_EXPONENTIAL_FUNCTION:
    m_Thetas.resize(3 + p);
    m_Thetas(0) = amplitude;
    m_Thetas(1) = defaultNugget;
    m_Thetas(2) = 2.0;
    for (int j = 0; j < p; ++j) {
      madai::Parameter & param = m_Parent->m_Parameters[j];
      madai::Distribution * priordist = param.m_PriorDistribution;
      m_Thetas(3+j) = scale * std::abs(priordist->GetPercentile(0.75)
                                     - priordist->GetPercentile(0.25));
    }
    break;
  case GaussianProcessEmulator::MATERN_32_FUNCTION:
    // fall through
  case GaussianProcessEmulator::MATERN_52_FUNCTION:
    m_Thetas.resize(3);
    m_Thetas(0) = amplitude;
    m_Thetas(1) = defaultNugget;
    {
      double min = std::numeric_limits< double >::max();
      for (int j = 0; j < p; ++j) {
        madai::Parameter & param = m_Parent->m_Parameters[j];
        madai::Distribution * priordist = param.m_PriorDistribution;
        double d = std::abs(priordist->GetPercentile(0.75)
                          - priordist->GetPercentile(0.25));
        if (d < min)
          min = d;
      }
      m_Thetas(2) = min * scale;
    }
    break;
  default:
    std::cerr << "Unknown covariance function.\n";
    return false;
  }
  return true;
}

bool GaussianProcessEmulator::PrincipalComponentDecompose(
    double fractionResolvingPower)
{
  if (fractionResolvingPower <= 0.0) {
    /* \todo error message to stderr */
    return false;
  }
  if (fractionResolvingPower > 1.0) {
    /* \todo error message to stderr */
    return false;
  }
  int t = m_NumberOutputs;
  int N = m_NumberTrainingPoints;

  // FIND PCA DECOMPOSIRION OF m_OutputValues - m_OutputMeans
  m_OutputMeans = m_OutputValues.colwise().mean();
  Eigen::MatrixXd Y_minus_means
    = m_OutputValues.rowwise() - (m_OutputValues.colwise().mean());


  Eigen::MatrixXd Y_standardized(N,t);
  for (int outputIndex = 0; outputIndex < t; ++outputIndex) {
    double oneOverUncertaintyScale
      = 1.0 / m_OutputUncertaintyScales(outputIndex);
    for (int pointIndex = 0; pointIndex < N; ++pointIndex) {
      Y_standardized(pointIndex, outputIndex)
        = oneOverUncertaintyScale * Y_minus_means(pointIndex, outputIndex);
    }
  }

  Eigen::MatrixXd Ycov
    = (1.0 / N ) * Y_standardized.transpose() * Y_standardized;

  Eigen::SelfAdjointEigenSolver< Eigen::MatrixXd > eigenSolver(Ycov);


  double resolving_power = 1.0;
  for (int i = t-1; i >= 0; --i)
    resolving_power *= std::sqrt(1.0 + eigenSolver.eigenvalues()(i));
  double target_resolving_power = resolving_power * fractionResolvingPower;

  resolving_power = 1.0;
  for (int i = t-1; i >= 0; --i) {
    resolving_power *= std::sqrt(1.0 + eigenSolver.eigenvalues()(i));
    if (resolving_power >= target_resolving_power) {
      m_NumberPCAOutputs = t - i;
      break;
    }
  }

  int r = m_NumberPCAOutputs;
  assert((r > 0) && (r <= t));
  m_PCAEigenvalues = eigenSolver.eigenvalues().tail(r);
  m_PCAEigenvectors = eigenSolver.eigenvectors().rightCols(r);

  Eigen::MatrixXd zMatrix = Y_standardized * m_PCAEigenvectors;
  m_PCADecomposedModels.resize(r);
  for (int i = 0; i < r; ++i) {
    SingleModel & m = m_PCADecomposedModels[i];
    m.m_Parent = this;
    m.m_ZValues = zMatrix.col(i);
  }
  return true;
}


/**
   Execute the model at an input point x.  Save a lot of time by not
   calculating the error. */
bool GaussianProcessEmulator::SingleModel::GetEmulatorOutputs (
    const std::vector< double > & x,
    double & mean) const {
  assert(m_RegressionOrder >= 0);
  // copy the point from vector<double> into VectorXd
  Eigen::VectorXd point = Eigen::Map<const Eigen::VectorXd>(&(x[0]),x.size());
  int N = m_Parent->m_NumberTrainingPoints;
  int p = m_Parent->m_NumberParameters;
  assert(p > 0);
  int F = 1 + (m_RegressionOrder * p);
  Eigen::MatrixXd & X = m_Parent->m_ParameterValues;
  Eigen::VectorXd kplus(N); // kplus is C(x,D)
  for (int j = 0; j < N; ++j) {
    Eigen::VectorXd xrow = X.row(j);
    double cov = this->CovarianceCalc(xrow, point);
    if(cov < 1e-10)
      cov = 0.0;
    kplus(j) = cov;
  }
  Eigen::VectorXd h_vector(F);
  MakeHVector(point,h_vector,m_RegressionOrder);

  // m_CInverse
  //   = CMatrix.ldlt().solve(Eigen::MatrixXd::Identity(N,N));
  // m_RegressionMatrix1
  //   = (HMatrix.transpose() * m_CInverse
  //       * HMatrix).ldlt().solve(Eigen::MatrixXd::Identity(F,F));
  // m_RegressionMatrix2
  //     = (m_CInverse * HMatrix).transpose(); //[FxN]
  // m_BetaVector
  //      = m_RegressionMatrix1 * HMatrix.transpose() * m_CInverse * m_ZValues;
  // m_GammaVector = m_CInverse * (m_ZValues - (HMatrix * m_BetaVector));
  mean = h_vector.dot(m_BetaVector) + kplus.dot(m_GammaVector);
  return true;
}
/**
   Execute the model at an input point x.  Save a lot of time by not
   calculating the covaraince error. */
bool GaussianProcessEmulator::GetEmulatorOutputs (
    const std::vector< double > & x,
    std::vector< double > & y) const
{
  if (m_Status != READY) {
    std::cerr << "GetEmulatorOutputs ERROR."
      " GaussianProcessEmulator is not ready.\n";
    return false;
  }
  Eigen::VectorXd mean_pca(m_NumberPCAOutputs);
  for (int i = 0; i < m_NumberPCAOutputs; ++i) {
    double d;
    if(! m_PCADecomposedModels[i].GetEmulatorOutputs(x, d))
      return false;
    mean_pca(i) = d;
  }
  y.resize(m_NumberOutputs);
  Eigen::Map< Eigen::VectorXd > mean(&(y[0]),m_NumberOutputs);
  mean = m_OutputMeans +
    m_OutputUncertaintyScales.cwiseProduct(m_PCAEigenvectors * mean_pca);
  return true;
}

/**
   Execute the model at an input point x.  Save a lot of time by not
   calculating the error. */
bool GaussianProcessEmulator::SingleModel
::GetEmulatorOutputsAndCovariance (
    const std::vector< double > & x,
    double & mean,
    double & variance) const {
  assert(m_RegressionOrder >= 0);
  Eigen::Map<const Eigen::VectorXd> point(&(x[0]),x.size());
  int N = m_Parent->m_NumberTrainingPoints;
  int p = m_Parent->m_NumberParameters;
  assert(p > 0);
  int F = 1 + (m_RegressionOrder * p);
  Eigen::MatrixXd & X = m_Parent->m_ParameterValues;
  Eigen::VectorXd kplus(N);
  // kplus is C(x,D)
  for (int j = 0; j < N; ++j) {
    Eigen::VectorXd xrow = X.row(j);
    double cov = this->CovarianceCalc(xrow, point);
    if(cov < 1e-10)
      cov = 0.0;
    kplus(j) = cov;
  }
  Eigen::VectorXd h_vector(F);
  h_vector(0) = 1.0;
  if (m_RegressionOrder > 0)
    h_vector.segment(1,p) = point;
  for (int i = 2; i < m_RegressionOrder; ++i) {
    h_vector.segment(1+(i*p),p)
      = h_vector.segment(1+((i-1)*p),p).cwiseProduct(point);
  }
  // m_CInverse = CMatrix.ldlt().solve(Eigen::MatrixXd::Identity(N,N));
  // m_RegressionMatrix
  //    = (HMatrix.transpose() * m_CInverse
  //         * HMatrix).ldlt().solve(Eigen::MatrixXd::Identity(F,F));
  // m_BetaVector
  //     = m_RegressionMatrix * HMatrix.transpose() * m_CInverse * m_ZValues;
  // m_GammaVector = m_CInverse * (m_ZValues - (HMatrix * m_BetaVector));
  mean = h_vector.dot(m_BetaVector) + kplus.dot(m_GammaVector);

  // Eigen::VectorXd  f = h_vector - kplus.dot(m_CInverse * h_vector);
  // variance = this->CovarianceCalc(point, point)
  //   - kplus.dot(m_CInverse * kplus) + f.dot(m_RegressionMatrix * f);

  // m_RegressionMatrix2
  //     = (m_CInverse * HMatrix).transpose(); //[FxN]

  Eigen::VectorXd  f = h_vector - (m_RegressionMatrix2 * kplus);

  variance = this->CovarianceCalc(point, point)
    - kplus.dot(m_CInverse * kplus) + f.dot(m_RegressionMatrix1 * f);
  return true;
}

/**
         Execute the model at an input point x.
         The covariance returned will be a flattened matrix */
bool GaussianProcessEmulator::GetEmulatorOutputsAndCovariance (
    const std::vector< double > & x,
    std::vector< double > & y,
    std::vector< double > & ycov) const {
  if (m_Status != READY)
    return false;

  Eigen::Map<const Eigen::VectorXd> point(&(x[0]),x.size());
  int t = m_NumberOutputs;
  Eigen::VectorXd mean_pca(m_NumberPCAOutputs);
  Eigen::VectorXd var_pca(m_NumberPCAOutputs);
  for (int i = 0; i < m_NumberPCAOutputs; ++i) {
    double smean, variance;
    if (! m_PCADecomposedModels[i].GetEmulatorOutputsAndCovariance(
            x, smean, variance))
      return false;
    mean_pca(i) = smean;
    var_pca(i) = variance;
  } // end-for(i < (m_NumberPCAOutputs))
  y.resize(t);
  ycov.resize(t * t);
  Eigen::Map< Eigen::VectorXd > mean(&(y[0]), t);
  Eigen::Map< Eigen::MatrixXd > covariance(&(ycov[0]), t, t);
  mean = m_OutputMeans +
    m_OutputUncertaintyScales.cwiseProduct(m_PCAEigenvectors * mean_pca);

  // var_pca = m_PCAEigenvalues.cwiseProduct(var_pca);
  // help!

  Eigen::MatrixXd uncertaintyScales
    = m_OutputUncertaintyScales * m_OutputUncertaintyScales.transpose();
  covariance
    = uncertaintyScales.cwiseProduct(
        m_PCAEigenvectors * var_pca.asDiagonal() *
        m_PCAEigenvectors.transpose());

  return true;
}


bool GaussianProcessEmulator::WritePCA( std::ostream & o ) const {
  o.precision(15);
  serializePCADecomposition(*this,o);
  return true;
}


bool GaussianProcessEmulator::Write(std::ostream & o) const {
  o.precision(17);
  serializeGaussianProcessEmulator(*this, o);
  return true;
}

/**
   Writes current state to file.  \returns true on sucess. */
bool GaussianProcessEmulator::PrintThetas(std::ostream & o) const {
  o.precision(17);
  serializeComments(m_Comments,o);
  o << "THETAS_FILE\n";
  o << "OUTPUT_MEANS\n";
  PrintVector(m_OutputMeans, o);
  o << "OUTPUT_UNCERTAINTY_SCALES\n";
  PrintVector(m_OutputUncertaintyScales, o);
  o << "OUTPUT_PCA_EIGENVALUES\n";
  PrintVector(m_PCAEigenvalues, o);
  o << "OUTPUT_PCA_EIGENVECTORS\n";
  PrintMatrix(m_PCAEigenvectors, o);
  o << "SUBMODELS\t"
    << m_NumberPCAOutputs << "\n\n";
  for (int i = 0; i < m_NumberPCAOutputs; ++i) {
    const SingleModel & m = m_PCADecomposedModels[i];
    o << "MODEL " << i << '\n';
    o << "COVARIANCE_FUNCTION\t"
      << GetCovarianceFunctionString(m.m_CovarianceFunction) << '\n';
    o << "REGRESSION_ORDER\t" << m.m_RegressionOrder << '\n';
    o << "THETAS\n";
    PrintVector(m.m_Thetas, o);
    o << "END_OF_MODEL\n\n";
  }
  o << "END_OF_FILE\n";
  return true;
}

/**
   \returns m_Status */
GaussianProcessEmulator::StatusType
GaussianProcessEmulator::GetStatus() const {
  return m_Status;
}

} // namespace madai
