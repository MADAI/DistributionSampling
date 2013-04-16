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

#include "GaussianProcessEmulatorSingleFileReader.h"

#include "GaussianDistribution.h"
#include "GaussianProcessEmulator.h"
#include "Paths.h"
#include "UniformDistribution.h"

#include <fstream>
#include <vector>


namespace madai {


GaussianProcessEmulatorSingleFileReader
::GaussianProcessEmulatorSingleFileReader()
{
}


GaussianProcessEmulatorSingleFileReader
::~GaussianProcessEmulatorSingleFileReader()
{
}


namespace {

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
  std::string PriorFileName = AnalysisDir + Paths::SEPARATOR +
    Paths::PARAMETER_PRIORS_FILE;
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


bool parseInteger( int & x, std::istream & input ) {
  if (! input.good()) return false;
  input >> x;
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


bool parsePCADecomposition(
    GaussianProcessEmulator & gpme,
    std::istream & input ) {
  parseComments(gpme.m_Comments,input);
  std::string word;
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
      gpme.m_NumberPCAOutputs = gpme.m_PCAEigenvalues.size();
    } else if (word == "OUTPUT_PCA_EIGENVECTORS") {
      if (! ReadMatrix(gpme.m_PCAEigenvectors, input)) {
        std::cerr << "parse error\n"; // \todo error message
        return false;
      }
    } else if (word == "END_OF_FILE") {

      // Now that we have the PCA data read in, build the Z vectors
      gpme.BuildZVectors();

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
    } else if (word == "OUTPUT_OBSERVED_VALUES") {
      if (! ReadVector(gpme.m_ObservedOutputValues, input)) {
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


} // end anonymous namespace


/**
    This loads a fully-ready-to-go GPEM
    \returns true on success. */
bool
GaussianProcessEmulatorSingleFileReader
::Load(GaussianProcessEmulator * gpe, std::istream & input)
{
  if (! parseGaussianProcessEmulator(*gpe, input)) {
    std::cerr << "FIXME need error message\n";
    return false;
  }
  // We are finished reading the input file.
  if (gpe->CheckStatus() != GaussianProcessEmulator::UNCACHED) {
    std::cerr << "FIXME status error\t" << stat(gpe->CheckStatus())
    << '\n';
    return false;
  }
  if (! gpe->MakeCache()) {
    std::cerr << "FIXME need error message\n";
    return false;
  }
  return true;
}


/**
    This takes an empty GPEM and loads training data.
    \returns true on success. */
bool
GaussianProcessEmulatorSingleFileReader
::LoadTrainingData(GaussianProcessEmulator * gpe, std::istream & input)
{
  if (! parseGaussianProcessEmulator(*gpe, input))
    return false;
  gpe->CheckStatus();
  return (gpe->CheckStatus() == GaussianProcessEmulator::UNTRAINED);
}

} // end namespace madai
