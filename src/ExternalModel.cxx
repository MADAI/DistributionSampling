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

#include <cassert>
#include <cctype>
#include <cstdio> // std::fgets, std::fscanf, et cetera.
#include <cstdlib> // EXIT_FAILURE
#include <fstream>
#include <string> // std::string

#include "ExternalModel.h"


namespace madai {

ExternalModel
::ExternalModel()
{
  // Mark the question and answer fields in the ProcessPipe as NULL so
  // that if they are not NULL at destruction we will know to close them.
  m_Process.question = NULL;
  m_Process.answer   = NULL;
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
}


/**
 * Loads a configuration from a file.  The format of the file is
 * defined by this function.  We'll lock it down later.
 */
ExternalModel::ErrorType
ExternalModel
::LoadConfigurationFile( const std::string fileName )
{
  return NO_ERROR;
}


static void discard_line( std::FILE * fp ) {
  static int buffersize = 1024;
  char buffer[buffersize];
  std::fgets( buffer, buffersize, fp );
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


static void eat_whitespace( std::istream & i ) {
  while ( true ) {
    if ( !std::isspace( i.peek() ) ) {
      return;
    }
    if ( i.get() == '\n' ) {
      return;
    }
  }
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


static bool discard_comments( std::istream & i, char comment_character ) {
  int c = i.peek();
  while ( i.good() && ( ( c == comment_character ) || ( c == '\n' ) ) ) {
    std::string s;
    std::getline( i, s );
    c = i.peek();
  }
}


ExternalModel::ErrorType
ExternalModel
::StartProcess( const std::string & processPath )
{
  char ** argv = new char*[2];
  argv[0] = new char[processPath.size()+1];
  strcpy( argv[0], processPath.c_str() );
  argv[0][processPath.size()] = '\0';
  argv[1] = NULL;

  // CreateProcessPipe returns EXIT_FAILURE on error, EXIT_SUCCESS otherwise
  if ( EXIT_FAILURE == CreateProcessPipe(&(m_Process), argv) ) {
    std::cerr << "CreateProcessPipe returned failure.\n";
    m_StateFlag = ERROR;
    return OTHER_ERROR;
  }

  if ( m_Process.answer == NULL || m_Process.question == NULL ) {
    std::cerr << "CreateProcessPipe returned NULL fileptrs.\n";
    m_StateFlag = ERROR;
    return OTHER_ERROR;
  }

  discard_comments( m_Process.answer, '#' );
  // allow comment lines to BEGIN the interactive process

  // Get parameters
  unsigned int numberOfParameters = 0;
  if ( 1 != std::fscanf( m_Process.answer, "%u", &numberOfParameters ) ) {
    std::cerr << "fscanf failure reading from the external process [1]\n";
    m_StateFlag = ERROR;
    return OTHER_ERROR;
  }

  eat_whitespace( m_Process.answer );
  for ( unsigned int i = 0; i < numberOfParameters; i++ ) {
    char buffer[256];
    std::fscanf(m_Process.answer, "%s", buffer);

    std::string parameterName( buffer );
    this->AddParameter( parameterName );
  }

  // Get output names
  unsigned int numberOfOutputs;
  if ( 1 != std::fscanf( m_Process.answer, "%d", &numberOfOutputs ) ) {
    std::cerr << "fscanf failure reading from the external process [2]\n";
    m_StateFlag = ERROR;
    return OTHER_ERROR;
  }

  eat_whitespace( m_Process.answer );
  for ( unsigned int i = 0; i < numberOfOutputs; i++ ) {
    char buffer[256];
    std::fscanf(m_Process.answer, "%s", buffer);

    std::string outputName( buffer );
    this->AddScalarOutputName( outputName );
  }

  /* We are now ready to go! */
  m_StateFlag = READY;

  return NO_ERROR;
}


/**
 * Get the scalar outputs from the model evaluated at x.  If an
 * error happens, the scalar output array will be left incomplete.
 */
ExternalModel::ErrorType
ExternalModel::GetScalarOutputs(
  const std::vector< double > & parameters,
  std::vector< double > & scalars ) const
{
  for ( std::vector< double >::const_iterator par_it = parameters.begin();
       par_it < parameters.end(); par_it++ ) {
    //FIXME to do: check against parameter range.
    std::fprintf( m_Process.question,"%.17lf\n", *par_it );
  }
  std::fflush( m_Process.question );

  for ( std::vector<double>::iterator ret_it = scalars.begin();
        ret_it < scalars.end(); ret_it++ ) {
    if (1 != fscanf(m_Process.answer, "%lf%*c", &(*ret_it))) {
      std::cerr << "interprocess communication error [cJ83A]\n";
      return OTHER_ERROR;
    }
  }

  return NO_ERROR;
}


// Not implemented yet.
// Get the likelihood and prior at the point theta
ExternalModel::ErrorType
ExternalModel::GetLikeAndPrior( const std::vector< double > & parameters,
                                double & Like, double & Prior ) const
{
  return OTHER_ERROR;
}

} // end namespace madai
