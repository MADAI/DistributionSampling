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

#include <cassert>
#include <cctype>
#include <cstdio> // std::fgets, std::fscanf, et cetera.
#include <cstdlib> // EXIT_FAILURE
#include <fstream>
#include <string> // std::string
#include <cstring> // std::strcmp

/** \todo Add macro defined at configure time that states whether
* signal.h is available to the compiler. */
#include <signal.h> // kill, SIGINT

#include "ExternalModel.h"
#include "GaussianDistribution.h"
#include "UniformDistribution.h"

#define str_equal(s1,s2) (std::strcmp ((s1), (s2)) == 0)

namespace madai {

ExternalModel
::ExternalModel() :
  m_CovarianceMode(NO_COVARIANCE)
{
  // Mark the question and answer fields in the ProcessPipe as NULL so
  // that if they are not NULL at destruction we will know to close them.
  m_Process.question = NULL;
  m_Process.answer   = NULL;
  m_Process.pid      = -1;
}


ExternalModel
::~ExternalModel()
{
  if ( m_Process.question != NULL ) {
    std::fclose( m_Process.question );
  }

  if ( m_Process.answer != NULL ) {
    std::fclose( m_Process.answer );
    // Hopefully the external process will clean itself up after its
    // stdin is closed for reading.
  }
  // If it doesn't clean itself up, we send a ctrl-c.
  if ( m_Process.pid != -1 ) {
    kill((pid_t)(m_Process.pid), SIGINT);
  }
}


static void discard_line( std::FILE * fp ) {
  const static int buffersize = 1024;
  char buffer[buffersize];
  if (NULL == std::fgets( buffer, buffersize, fp ) && !feof(fp)) {
    std::cerr << "ExternalModel: discard_line(): error in fgets()\n";
  }
}


static bool discard_comments( std::FILE * fp, char comment_character ) {
  int c = std::getc( fp );
  if ( (c == EOF) || std::ferror( fp ) ) {
    std::cerr << "premature end of file:(\n";
    return false;
  }
  while ( c == comment_character ) {
    discard_line(fp);
    c = std::getc(fp);
  }
  if ( EOF == std::ungetc(c, fp) ) {
    std::cerr << "ungetc error :(\n";
    return false;
  }
  return true;
}


static void eat_whitespace( std::FILE * fp ) {
  while ( true ) {
    int c = std::fgetc( fp );
    if ( !std::isspace( c ) ) {
      std::ungetc( c, fp );
      return;
    }
    if ( c == '\n' ) {
      return;
    }
  }
}


ExternalModel::ErrorType
ExternalModel
::StartProcess( const std::string & processPath,
                const std::vector< std::string > & arguments )
{
  // Create array of C strings from arguments array
  char ** argv = new char*[arguments.size() + 1];

  // First argument is expected to be the executable
  argv[0] = new char[processPath.size()+1];
  strcpy( argv[0], processPath.c_str() );

  for ( size_t i = 0; i < arguments.size(); ++i ) {
    argv[i+1] = new char[arguments[i].size()+1];
    strcpy( argv[i+1], arguments[i].c_str() );
  }
  argv[arguments.size()+1] = NULL; // NULL-terminated array

  // CreateProcessPipe returns EXIT_FAILURE on error, EXIT_SUCCESS otherwise
  int createError = CreateProcessPipe( &(m_Process), argv );

  // Free up the argv arrays
  for ( size_t i = 0; i <= arguments.size(); ++i ) {
    delete[] argv[i];
  }
  delete[] argv;

  // Have a pessimistic outlook
  m_StateFlag = ERROR;

  // Now check for an error in the ProcessPipe creation
  if ( EXIT_FAILURE == createError ) {
    std::cerr << "CreateProcessPipe returned failure.\n";
    return OTHER_ERROR;
  }

  if ( m_Process.answer == NULL || m_Process.question == NULL ) {
    std::cerr << "CreateProcessPipe returned NULL fileptrs.\n";
    return OTHER_ERROR;
  }

  discard_comments( m_Process.answer, '#' );
  // allow comment lines to BEGIN the interactive process

  // Get the version of the protocol we are speaking
  char buffer[4096];
  int versionNumber = 1;
  if ( 2 != std::fscanf( m_Process.answer, "%4095s %d",
                         buffer, &versionNumber) ) {
    std::cerr
      << "fscanf failure reading version number from external process\n";
    return OTHER_ERROR;
  }

  if (! str_equal(buffer, "VERSION")) {
    std::cerr << "failure reading version number from external process\n";
    return OTHER_ERROR;
  }
  if (versionNumber != 1) {
    std::cerr << "Unknown interface version\n";
    return OTHER_ERROR;
  }

  if ( 1 != std::fscanf( m_Process.answer, "%4095s", buffer) ) {
    std::cerr << "fscanf failure @ PARAMETERS\n";
    return OTHER_ERROR;
  }

  if (! str_equal(buffer, "PARAMETERS")) {
    std::cerr << "syntax error @ PARAMETERS\n";
    return OTHER_ERROR;
  }

  // Get parameters
  unsigned int numberOfParameters = 0;
  if ( 1 != std::fscanf( m_Process.answer, "%u", &numberOfParameters ) ) {
    std::cerr << "fscanf failure reading from the external process [1]\n";
    return OTHER_ERROR;
  }
  assert(numberOfParameters > 0);
  eat_whitespace( m_Process.answer );
  for ( unsigned int i = 0; i < numberOfParameters; i++ ) {
    if ( 1 != std::fscanf(m_Process.answer, "%4095s", buffer)) {
      std::cerr << "fscanf failure @ parameterName\n";
      return OTHER_ERROR;
    }

    std::string parameterName( buffer );

    eat_whitespace( m_Process.answer );
    if ( 1 != std::fscanf( m_Process.answer, "%4095s", buffer ) ) {
      std::cerr << "fscanf failure @ parameterName\n";
      return OTHER_ERROR;
    }

    if ( str_equal( buffer, "UNIFORM" ) ) {
      double parameterMin, parameterMax;
      if ( 1 != std::fscanf(m_Process.answer, "%lf", &parameterMin)) {
        std::cerr << "fscanf failure @ parametermin\n";
        return OTHER_ERROR;
      }
      if ( 1 != std::fscanf(m_Process.answer, "%lf", &parameterMax)) {
        std::cerr << "fscanf failure @ parametermin\n";
        return OTHER_ERROR;
      }

      UniformDistribution uniformDistribution;
      uniformDistribution.SetMinimum( parameterMin );
      uniformDistribution.SetMaximum( parameterMax );

      this->AddParameter( parameterName, uniformDistribution );

    } else if ( str_equal( buffer, "GAUSSIAN" ) ) {
      double parameterMean, parameterStandardDeviation;
      if ( 1 != std::fscanf( m_Process.answer, "%lf", &parameterMean ) ) {
        std::cerr << "fscanf failure @ parameterMean\n";
        return OTHER_ERROR;
      }
      if ( 1 != std::fscanf( m_Process.answer, "%lf", &parameterStandardDeviation ) ) {
        std::cerr << "fscanf failure @ parameterStandardDeviation\n";
        return OTHER_ERROR;
      }

      GaussianDistribution gaussianDistribution;
      gaussianDistribution.SetMean( parameterMean );
      gaussianDistribution.SetStandardDeviation( parameterStandardDeviation );

      this->AddParameter( parameterName, gaussianDistribution );
    } else {
      std::cerr << "Unknown parameter prior distribution type '"
                << buffer << "'\n";
      return OTHER_ERROR;
    }
  }

  if ( 1 != std::fscanf( m_Process.answer, "%4095s", buffer) ) {
    std::cerr << "fscanf failure @ OUTPUTS\n";
    return OTHER_ERROR;
  }

  if (! str_equal(buffer, "OUTPUTS")) {
    std::cerr << "syntax error @ OUTPUTS\n";
    return OTHER_ERROR;
  }

  // Get output names
  unsigned int numberOfOutputs;
  if ( 1 != std::fscanf( m_Process.answer, "%u", &numberOfOutputs ) ) {
    std::cerr << "fscanf failure reading from the external process [2]\n";
    return OTHER_ERROR;
  }

  eat_whitespace( m_Process.answer );
  for ( unsigned int i = 0; i < numberOfOutputs; i++ ) {
    if ( 1 != std::fscanf(m_Process.answer, "%4095s", buffer)) {
      std::cerr << "fscanf failure @ outputName.\n";
      return OTHER_ERROR;
    }
    std::string outputName( buffer );
    this->AddScalarOutputName( outputName );
  }
  eat_whitespace( m_Process.answer );

  if ( 1 != std::fscanf( m_Process.answer, "%4095s", buffer) ) {
    std::cerr << "fscanf failure @ NEXT\n";
    return OTHER_ERROR;
  }

  // covariance;

  if (str_equal(buffer, "COVARIANCE")) {
    if ( 1 != std::fscanf( m_Process.answer, "%4095s", buffer) ) {
      std::cerr << "fscanf failure @ TRIANGULAR_MATRIX\n";
      return OTHER_ERROR;
    }
    if (str_equal(buffer, "FULL_MATRIX")) {
      this->m_CovarianceMode = FULL_MATRIX_COVARIANCE;
      unsigned int covarianceSize;
      if ( 1 != std::fscanf( m_Process.answer, "%u", &covarianceSize ) ) {
        std::cerr << "fscanf failure @ covarianceSize.\n";
        return OTHER_ERROR;
      }
      if (covarianceSize != (numberOfOutputs * numberOfOutputs)) {
        std::cerr << "full covariance matrix wrong size\n";
        return OTHER_ERROR;
      }
    } else if (str_equal(buffer, "TRIANGULAR_MATRIX")) {
      this->m_CovarianceMode = TRIANGULAR_COVARIANCE;
      unsigned int covarianceSize;
      if ( 1 != std::fscanf( m_Process.answer, "%u", &covarianceSize ) ) {
        std::cerr << "fscanf failure @ covarianceSize.\n";
        return OTHER_ERROR;
      }
      if (covarianceSize != ((numberOfOutputs * (numberOfOutputs + 1)) / 2)) {
        std::cerr << "triangular covariance matrix wrong size\n";
        return OTHER_ERROR;
      }
    } else {
      std::cerr << "unsupported covariance matrix format\n";
      return OTHER_ERROR;
    }
    if ( 1 != std::fscanf( m_Process.answer, "%4095s", buffer) ) {
      std::cerr << "fscanf failure @ COVNEXT\n";
      return OTHER_ERROR;
    }
  } else if (str_equal(buffer, "VARIANCE")) {
    unsigned int varianceSize;
    if ( 1 != std::fscanf( m_Process.answer, "%u", &varianceSize ) ) {
      std::cerr << "fscanf failure @ varianceSize.\n";
      return OTHER_ERROR;
    }
    if (varianceSize != numberOfOutputs) {
      std::cerr << "wrong  variance size.\n";
      return OTHER_ERROR;
    }

    this->m_CovarianceMode = DIAGONAL_MATRIX_COVARIANCE;

    if ( 1 != std::fscanf( m_Process.answer, "%4095s", buffer) ) {
      std::cerr << "fscanf failure @ VARNEXT\n";
      return OTHER_ERROR;
    }
  } else {
    this->m_CovarianceMode = NO_COVARIANCE;
  }

  if (! str_equal(buffer, "END_OF_HEADER")) {
    std::cerr << "syntax error @ END_OF_HEADER\n";
    return OTHER_ERROR;
  }

  // We are now ready to go!
  m_StateFlag = READY;

  return NO_ERROR;
}


ExternalModel::ErrorType
ExternalModel
::StopProcess()
{
  if ( !this->IsReady() ) {
    return OTHER_ERROR;
  }

  // Send the STOP message
  std::fprintf( m_Process.question, "STOP\n" );
  std::fflush( m_Process.question );

  // Don't expect an answer

  return NO_ERROR;
}


inline static bool read_double(std::FILE * fptr, double * d) {
  return (1 == fscanf(fptr, "%lf%*c", d));
}

/** Get the scalar outputs from the model evaluated at x.
 *
 * \todo Consider using this as default implementation in Model */
ExternalModel::ErrorType
ExternalModel
::GetScalarOutputs( const std::vector< double > & parameters,
                    std::vector< double > & scalars ) const {
  std::vector< double > scalarCovariance;
  return this->GetScalarOutputsAndCovariance(parameters, scalars, scalarCovariance);
}

/**
 * Get the scalar outputs from the model evaluated at x.  If an
 * error happens, the scalar output array will be left incomplete.
 */
ExternalModel::ErrorType
ExternalModel
::GetScalarOutputsAndCovariance(
      const std::vector< double > & parameters,
      std::vector< double > & scalars,
      std::vector< double > & scalarCovariance) const
{
  if ( !this->IsReady() ) {
    return OTHER_ERROR;
  }
  if (parameters.size() != this->GetNumberOfParameters())
    return WRONG_VECTOR_LENGTH;

  ProcessPipe & proc = (const_cast< ExternalModel * >(this))->m_Process;

  for ( std::vector< double >::const_iterator par_it = parameters.begin();
       par_it < parameters.end(); par_it++ ) {
    //FIXME to do: check against parameter range.
    std::fprintf( proc.question,"%.17f\n", *par_it );
  }
  std::fflush( proc.question );

  size_t t = this->GetNumberOfScalarOutputs();
  scalars.resize(t);

  for ( std::vector<double>::iterator ret_it = scalars.begin();
        ret_it < scalars.end(); ret_it++ ) {
    if (! read_double(proc.answer, &(*ret_it))) {
      std::cerr << "interprocess communication error\n";
      return OTHER_ERROR;
    }
  }
  switch (this->m_CovarianceMode) {

  case NO_COVARIANCE:
    scalarCovariance.clear();
    break;

  case TRIANGULAR_COVARIANCE:
    scalarCovariance.resize(t * t);
    for (size_t i = 0; i < t; ++i) {
      for (size_t j = i; j < t; ++j) {
        double d;
        if (! read_double(proc.answer, &d)) {
          std::cerr << "interprocess communication error\n";
          return OTHER_ERROR;
        }
        if (i==j)
          scalarCovariance[i*(1+t)] = d;
        else
          scalarCovariance[i+(t*j)] = scalarCovariance[j+(t*i)] = d;
      }
    }
    break;

  case FULL_MATRIX_COVARIANCE:
    scalarCovariance.resize(t * t);
    for (size_t i = 0; i < t; ++i) {
      for (size_t j = 0; j < t; ++j) {
        double * d = &(scalarCovariance[i+(t*j)]);
        if (! read_double(proc.answer, d)) {
          std::cerr << "interprocess communication error\n";
          return OTHER_ERROR;
        }
      }
    }
    break;

  case DIAGONAL_MATRIX_COVARIANCE: /* VARIANCE only */
    scalarCovariance.assign(t * t,0.0);
    for ( size_t i = 0; i < t; ++i) {
      if (! read_double(proc.answer, &(scalarCovariance[i*(1+t)]))) {
        std::cerr << "interprocess communication error\n";
        return OTHER_ERROR;
      }
    }
    break;

  } // end switch
  return NO_ERROR;
}

} // end namespace madai
