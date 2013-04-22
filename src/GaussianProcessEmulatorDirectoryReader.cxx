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

#include "GaussianProcessEmulatorDirectoryReader.h"

#include "GaussianDistribution.h"
#include "GaussianProcessEmulator.h"
#include "Paths.h"
#include "UniformDistribution.h"

#include <madaisys/Directory.hxx>
#include <madaisys/SystemTools.hxx>

#include <fstream>
#include <iostream> // std::cerr
#include <algorithm>  // std::find


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


bool parseInteger( int & x, std::istream & input ) {
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
    assert (index < sv.size());
    return true;
  }
}

/**
   Read the parameter_priors.dat file in statistical_analysis. */
bool parseExperimentalResults(
    GaussianProcessEmulator & gpe,
    std::string ExperimentalResultsDirectory ) {
  const std::vector< std::string > & outputNames = gpe.m_OutputNames;
  int numberOutputs = gpe.m_NumberOutputs;
  if (numberOutputs != outputNames.size()) {
    std::cerr << "numberOutputs != outputNames.size()";
    return false;
  }
  std::string ExperimentalResultsFileName
    = ExperimentalResultsDirectory + Paths::SEPARATOR + Paths::RESULTS_FILE;
  std::ifstream results_file( ExperimentalResultsFileName.c_str() );
  if ( !results_file.good() ) {
    std::cerr << "Error opening " << ExperimentalResultsFileName << '\n';
    return false;
  }
  // default values.
  gpe.m_OutputUncertaintyScales = Eigen::VectorXd::Constant(numberOutputs, 1.0);
  gpe.m_ObservedOutputValues = Eigen::VectorXd::Zero(numberOutputs);
  while ( !results_file.eof() ) {
    while ( results_file.peek() == '#' ) {// Disregard comments, go to next output
      std::string temp;
      std::getline( results_file, temp );
    }
    std::string name;
    double value, error;
    results_file >> name >> value >> error;
    int index;
    if (getIndex(outputNames, name, index)) {
      gpe.m_ObservedOutputValues(index) = value;
      gpe.m_OutputUncertaintyScales(index) = error;
    }
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
    if ( !(input >> type >> name >> dist_vals[0] >> dist_vals[1]) )
      break;
    std::transform( type.begin(), type.end(), type.begin(), ::tolower );
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


bool parseOutputs(
    std::vector< std::string > & outputNames,
    int & numberOutputs,
    std::string AnalysisDir ) {
  // First cehck to see if file exists
  std::string ObservablesFileName = AnalysisDir + Paths::SEPARATOR +
    Paths::OBSERVABLE_NAMES_FILE;
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


bool parseNumberOfModelRuns( int & x, std::string ModelOutDir ) {

  madaisys::Directory directory;
  if ( !directory.Load( ModelOutDir.c_str() ) ) {
    return false;
  }

  unsigned int run_counter = 0;

  for ( unsigned long i = 0; i < directory.GetNumberOfFiles(); ++i ) {
    int dummy;
    if ( sscanf( directory.GetFile( i ), "run%d", &dummy ) == 1 ) {
      run_counter++;
    }
  }

  x = run_counter;
  assert ( x > 0 );

  return true;
}


template < typename TDerived >
inline bool parseParameterAndOutputValues(
    const Eigen::MatrixBase< TDerived > & m_,
    const Eigen::MatrixBase< TDerived > & m2_,
    const Eigen::MatrixBase< TDerived > & m3_,
    std::string ModelOutDir,
    unsigned int numberTrainingPoints,
    std::vector< madai::Parameter > parameters,
    std::vector< std::string > outputNames,
    bool verbose ) {
  // Get the list of directories in model_outputs/

  madaisys::Directory directory;
  if ( !directory.Load( ModelOutDir.c_str() ) ) {
    return false;
  }

  std::vector< std::string > runDirectories;
  for ( unsigned long i = 0; i < directory.GetNumberOfFiles(); ++i ) {
    std::string fileName( directory.GetFile( i ) );
    std::string filePath( ModelOutDir + fileName );
    if ( fileName.find_first_of( "run" ) == 0 ) {
      runDirectories.push_back( fileName );
    }
  }
  std::sort( runDirectories.begin(), runDirectories.end() );

  int p = parameters.size();
  int t = outputNames.size();
  // Copy m_
  Eigen::MatrixBase< TDerived > & m
  = const_cast< Eigen::MatrixBase< TDerived > & >(m_);
  // Copy m2_
  Eigen::MatrixBase< TDerived > & m2
  = const_cast< Eigen::MatrixBase< TDerived > & >(m2_);
  // Copy m3_
  Eigen::MatrixBase< TDerived > & m3
  = const_cast< Eigen::MatrixBase< TDerived > & >(m3_);
  m.derived().resize( numberTrainingPoints, p );
  m2.derived().resize( numberTrainingPoints, t );
  m3.derived().resize( t, 1 );
  unsigned int run_counter = 0;

  double* avgUnc = new double[t];

  for ( size_t i = 0; i < runDirectories.size(); ++i ) {
    std::string dir_name( runDirectories[i] );
    if ( verbose )
      std::cout << "Run directory name: '" << dir_name << "'\n";

    if ( dir_name.find_first_of( "run" ) == 0 ) {
      // Open the parameters.dat file
      std::string par_file_name = ModelOutDir + dir_name +
        Paths::SEPARATOR + Paths::PARAMETERS_FILE;
      std::ifstream parfile ( par_file_name.c_str() );
      if ( !parfile.good() ) return false;
      if ( verbose )
        std::cout << "Opened file '" << par_file_name << "'\n";
      while ( !parfile.eof() ) {
        while ( parfile.peek() == '#' ) {
          std::string tline;
          std::getline( parfile, tline );
        }
        std::string name;
        parfile >> name;
        for ( unsigned int i = 0; i < p; i++ ) {
          if ( name == parameters[i].m_Name ) {
            parfile >> m( run_counter, i);
            if ( verbose )
              std::cout << "Parsed parameter '" << name << "' with value "
                        << m( run_counter, i ) << std::endl;
            break;
          }
        }
      }
      parfile.close();
      // Open the results.dat file
      std::string results_file_name = ModelOutDir + dir_name +
        Paths::SEPARATOR + Paths::RESULTS_FILE;
      std::ifstream results_file ( results_file_name.c_str() );
      if ( !results_file.good() ) return false;
      if ( verbose )
        std::cout << "Opened file " << results_file_name << "'\n";

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
            if ( verbose )
              std::cout << "Parsed output '" << name << "' with value "
                        << m2( run_counter, i ) << std::endl;
            if ( NVal == 3 ) {
              double ModelUnc;
              results_file >> ModelUnc;
              avgUnc[i] += ModelUnc;
            } else if ( NVal == 2 ) {
              avgUnc[i] += 1.0; // default model uncertainty is 1
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

  for ( unsigned int i = 0; i < t; i++ ) {
    m3( i, 0 ) = avgUnc[i] / double( run_counter );
  }

  return true;
}


bool parseModelDataDirectoryStructure(
    GaussianProcessEmulator & gpme,
    std::string Model_Outs_Dir,
    std::string Stat_Anal_Dir,
    bool verbose) {
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
  Eigen::MatrixXd TMat( gpme.m_NumberOutputs, 1 );
  if ( !parseParameterAndOutputValues(
          gpme.m_ParameterValues, gpme.m_OutputValues, TMat, Model_Outs_Dir,
          gpme.m_NumberTrainingPoints, gpme.m_Parameters, gpme.m_OutputNames,
          verbose ) ) {
    std::cerr << "parse Parameter and Output Values error\n";
    return false;
  }
  gpme.m_OutputUncertaintyScales = TMat.col(0);
  return true;
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
      gpme.m_PCADecomposedModels.resize( gpme.m_NumberPCAOutputs );
    } else if (word == "OUTPUT_PCA_EIGENVECTORS") {
      if (! ReadMatrix(gpme.m_PCAEigenvectors, input)) {
        std::cerr << "parse error\n"; // \todo error message
        return false;
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
    std::string TopDirectory) {
  std::string EmulatorFile = TopDirectory + Paths::SEPARATOR +
    Paths::STATISTICAL_ANALYSIS_DIRECTORY +
    Paths::SEPARATOR + Paths::EMULATOR_STATE_FILE;
  std::ifstream input( EmulatorFile.c_str() );
  parseComments(gpme.m_Comments,input);
  std::string word;
  while (input.good()) {
    if (! input.good()) return false;
    input >> word;
    if (word == "SUBMODELS") {
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

  return true;
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
  This takes an empty GPEM and loads training data.
  \returns true on success. */
bool
GaussianProcessEmulatorDirectoryReader
::LoadTrainingData(GaussianProcessEmulator * gpe, std::string TopDirectory)
{
  std::string MOD = TopDirectory + Paths::SEPARATOR +
    Paths::MODEL_OUTPUT_DIRECTORY + Paths::SEPARATOR;
  std::string SAD = TopDirectory + Paths::SEPARATOR +
    Paths::STATISTICAL_ANALYSIS_DIRECTORY + Paths::SEPARATOR;
  if ( !parseModelDataDirectoryStructure(*gpe, MOD, SAD, m_Verbose ) )
    return false;

  std::string ExperimentalResultsDirectory
    = TopDirectory + Paths::SEPARATOR + Paths::EXPERIMENTAL_RESULTS_DIRECTORY;
  if (! parseExperimentalResults( *gpe, ExperimentalResultsDirectory )) {
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
::LoadPCA(GaussianProcessEmulator * gpe, std::string TopDirectory)
{
  std::string PCAFile = TopDirectory + Paths::SEPARATOR +
    Paths::STATISTICAL_ANALYSIS_DIRECTORY + Paths::SEPARATOR +
    Paths::PCA_DECOMPOSITION_FILE;
  std::ifstream input( PCAFile.c_str() );
  if ( !parsePCADecomposition(*gpe, input) ) {
    std::cerr << "Error parsing PCA data.\n";
    return false;
  }

  // We are finished reading the input files.
  return (gpe->CheckStatus() == GaussianProcessEmulator::UNTRAINED);
}


/**
  This takes a GPEM and loads the emulator specific
  data (submodels with their thetas).
  \returns true on success. */
bool
GaussianProcessEmulatorDirectoryReader
::LoadEmulator(GaussianProcessEmulator * gpe, std::string TopDirectory)
{
  if ( !parseGaussianProcessEmulator(*gpe, TopDirectory) ) {
    std::cerr << "Error parsing gaussian process emulator.\n";
    return false;
  }
  // We are finished reading the input files.
  if ( gpe->CheckStatus() != GaussianProcessEmulator::UNCACHED ) {
    std::cerr << "Emulator already cached.\n";
    std::cerr << stat(gpe->CheckStatus()) << "\n";
    return false;
  }
  if ( !gpe->MakeCache() ) {
    std::cerr << "Error while makeing cache.\n";
    return false;
  }
  return true;
}

} // end namespace madai
