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
::ExternalModel() :
  m_NumberOfParameters( 0 ),
  m_NumberOfOutputs( 0 )
{
  this->m_Process.question = NULL; // constructor must do this
  // so that we know whether the destructor must act.
  this->m_Process.answer = NULL; // construcotr must do this.
  this->m_StateFlag = UNINITIALIZED;
}


ExternalModel
::ExternalModel( const std::string & m_ConfigurationFileName )
{
  this->m_Process.question = NULL; // construcotr must do this
  // so that we know whether the destructor must act.
  this->m_Process.answer = NULL; // construcotr must do this.
  this->m_StateFlag = UNINITIALIZED;

  this->LoadConfigurationFile(m_ConfigurationFileName);
}


ExternalModel
::~ExternalModel()
{
  if ( this->m_Process.question != NULL ) {
    std::fclose( this->m_Process.question );
  }

  if ( this->m_Process.answer != NULL ) {
    std::fclose( this->m_Process.answer );
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
  this->m_ConfigurationFileName = fileName; // keep a copy of the file name
                                            // just in case we need it.
  std::ifstream configFile( fileName.c_str() );
  ErrorType r = this->LoadConfigurationFile( configFile );
  configFile.close();
  return r;
}


void discard_line( std::FILE * fp ) {
  static int buffersize = 1024;
  char buffer[buffersize];
  std::fgets( buffer, buffersize, fp );
}


bool discard_comments( std::FILE * fp, char comment_character ) {
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


void eat_whitespace( std::istream & i ) {
  while ( true ) {
    if ( !std::isspace( i.peek() ) ) {
      return;
    }
    if ( i.get() == '\n' ) {
      return;
    }
  }
}


void eat_whitespace( std::FILE * fp ) {
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


bool discard_comments( std::istream & i, char comment_character ) {
  int c = i.peek();
  while ( i.good() && ( ( c == comment_character ) || ( c == '\n' ) ) ) {
    std::string s;
    std::getline( i, s );
    c = i.peek();
  }
}


ExternalModel::ErrorType
ExternalModel::LoadConfigurationFile( std::istream & configFile )
{
  discard_comments( configFile, '#' );
  configFile >> this->m_NumberOfParameters;
  if (this->m_NumberOfParameters < 1) {
    std::cerr << "bad value [YRDR]\n"; // 1g6o8duPJOAzMJgo
    this->m_StateFlag = ERROR;
    return OTHER_ERROR;
  }
  eat_whitespace( configFile );
  //std::cerr << this->m_NumberOfParameters << '\n';
  this->m_Parameters.reserve( this->m_NumberOfParameters );
  for ( unsigned int i = 0; i < this->m_NumberOfParameters; i++ ) {
    std::string s;
    double minimum = 0.0, maximum = 0.0;
    std::getline( configFile, s );
    configFile >> minimum >> maximum;
    if ( minimum >= maximum ) {
      std::cerr << "bad range [Tw4X]: " << i << ' '<<minimum << ':' << maximum<<"\n";
      this->m_StateFlag = ERROR;
      return OTHER_ERROR;
    }
    eat_whitespace( configFile );
    this->m_Parameters.push_back( Parameter( s, minimum, maximum ) );
    //std::cerr << s << '\n';
  }
  configFile >> this->m_NumberOfOutputs;
  if ( this->m_NumberOfOutputs < 1 ) {
    std::cerr << "bad value [YRDR]\n"; // 1g6o8duPJOAzMJgo
    this->m_StateFlag = ERROR;
    return OTHER_ERROR;
  }
  eat_whitespace( configFile );
  //std::cerr << this->m_NumberOfOutputs << '\n';
  this->m_ScalarOutputNames.reserve( this->m_NumberOfParameters );
  for ( unsigned int i = 0; i < this->m_NumberOfOutputs; i++ ) {
    std::string s;
    std::getline( configFile, s );
    this->m_ScalarOutputNames.push_back( s );
    //std::cerr << s << '\n';
  }
  unsigned int command_line_length;
  configFile >> command_line_length;
  eat_whitespace( configFile );
  //  std::cerr << command_line_length << '\n';
  assert( command_line_length > 0 );
  char ** argv = new char* [command_line_length + 1];
  argv[command_line_length] = NULL; // termination signal for exec*();

  for ( unsigned int i = 0; i < command_line_length; i++ ) {
    std::string s;
    std::getline( configFile, s );
    unsigned int stringsize = s.size();
    argv[i] = new char[stringsize + 1];
    s.copy( argv[i], stringsize );
    argv[i][stringsize] = '\0'; //nil terminated c-string.
  }

  /*
        We are now done reading configuration settings.  We will now
        open a pipe to the emulator and leave it going
  */

  /** function returns EXIT_FAILURE on error, EXIT_SUCCESS otherwise */
  if ( EXIT_FAILURE == create_process_pipe(&(this->m_Process), argv) ) {
    std::cerr << "create_process_pipe returned failure.\n";
    this->m_StateFlag = ERROR;
    return OTHER_ERROR;
  }
  for ( unsigned int i = 0; i < command_line_length; i++ ) {
    delete argv[i];
  }
  delete[] argv;

  if ( this->m_Process.answer == NULL || this->m_Process.question == NULL ) {
    std::cerr << "create_process_pipe returned NULL fileptrs.\n";
    this->m_StateFlag = ERROR;
    return OTHER_ERROR;
  }

  discard_comments( this->m_Process.answer, '#' );
  // allow comment lines to BEGIN the interactive process

  unsigned int n;
  if ( 1 != std::fscanf(this->m_Process.answer,"%u",&n) ) {
    std::cerr << "fscanf failure reading from the external process [1]\n";
    this->m_StateFlag = ERROR;
    return OTHER_ERROR;
  }
  if ( n != this->m_NumberOfParameters ) {
    std::cerr << "m_NumberOfParameters mismatch\n";
    this->m_StateFlag = ERROR;
    return OTHER_ERROR;
  }
  eat_whitespace( this->m_Process.answer );
  for ( unsigned int i = 0; i < this->m_NumberOfParameters; i++ ) {
    discard_line( this->m_Process.answer );
    // read but don't do anything with parameter names.
    // THINK ABOUT where is best to define these?
  }
  if ( 1 != std::fscanf(this->m_Process.answer,"%d", &n ) ) {
    std::cerr << "fscanf failure reading from the external process [2]\n";
    this->m_StateFlag = ERROR;
    return OTHER_ERROR;
  }
  if ( n != this->m_NumberOfOutputs ) {
    std::cerr << "m_NumberOfOutputs mismatch";
    this->m_StateFlag = ERROR;
    return OTHER_ERROR;
  }
  eat_whitespace( this->m_Process.answer );
  for ( unsigned int i = 0; i < this->m_NumberOfOutputs; i++ ) {
    discard_line( this->m_Process.answer );
  }

  /* We are now ready to go! */
  this->m_StateFlag = READY;
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
    std::fprintf( this->m_Process.question,"%.17lf\n", *par_it );
  }
  std::fflush( this->m_Process.question );
  for ( std::vector<double>::iterator ret_it = scalars.begin();
        ret_it < scalars.end(); ret_it++ ) {
    if (1 != fscanf(this->m_Process.answer, "%lf%*c", &(*ret_it))) {
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
