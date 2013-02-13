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

#include "MultiModel.h"

#include "LikelihoodDistribution.h"
#include "PriorDistribution.h"

#include <cctype>


namespace madai {


void
MultiModel
::discard_line( std::FILE * fp )
{
  static int buffersize = 1024;
  char buffer[buffersize];
  std::fgets( buffer, buffersize, fp );
}


bool
MultiModel
::discard_comments( std::FILE * fp, char comment_character )
{
  int c = std::getc( fp );
  if ( ( c == EOF ) || std::ferror( fp ) ) {
    std::cerr << "premature end of file:(\n";
    return false;
  }
  while ( c == comment_character ) {
    discard_line( fp );
    c = std::getc( fp );
  }
  if ( EOF == std::ungetc( c, fp ) ) {
    std::cerr << "ungetc error :(\n";
    return false;
  }
  return true;
}


void
MultiModel
::eat_whitespace( std::istream & i )
{
  while ( true ) {
    if ( !std::isspace( i.peek() ) ) {
      return;
    }
  if ( i.get() == '\n' )
    return;
  }
}


void
MultiModel
::eat_whitespace( std::FILE * fp )
{
  while ( true ) {
    int c = std::fgetc( fp );
    if ( !std::isspace( c ) ) {
      std::ungetc( c, fp );
      return;
    }
    if ( c == '\n' )
      return;
  }
}


bool
MultiModel
::discard_comments( std::istream & i, char comment_character )
{
  int c = i.peek();
  while ( i.good() && ( ( c == comment_character ) || ( c == '\n') ) ) {
    std::string s;
    std::getline( i, s );
    c = i.peek();
  }
}


MultiModel
::MultiModel() :
  m_Likelihood( NULL ),
  m_Prior( NULL ),
  m_PrescaledParams( false ),
  m_UseEmulator( false ),
  m_ProcessPipe( false )
{
}


MultiModel
::~MultiModel()
{
  if ( m_Likelihood != NULL ) {
    delete m_Likelihood;
  }
  if ( m_Prior != NULL ) {
    delete m_Prior;
  }
}


MultiModel::ErrorType
MultiModel
::LoadConfiguration( std::string info_dir )
{
  m_DirectoryName = info_dir;
  m_ParameterFile = info_dir + "/defaultpars/";
  std::cout << "There should be something here ->" << m_ParameterFile << "<-" << std::endl;
  std::cout << "In config: " << m_ParameterFile << std::endl;
  m_ParameterFileName = m_ParameterFile + "/mcmc.param";
  std::cout << "Reading in " << m_ParameterFileName << std::endl;

  parameter::ReadParsFromFile( m_ParameterMap, m_ParameterFileName.c_str() );
  m_LogLike     = parameter::getB( m_ParameterMap, "LOGLIKE", true );
  m_ModelType   = parameter::getS( m_ParameterMap,"MODEL","NOMODEL" );
  m_ProcessPipe = parameter::getB( m_ParameterMap, "PROCESS_PIPE", false );
  m_UseEmulator = parameter::getB( m_ParameterMap, "USE_EMULATOR", false );
  m_Optimizer   = parameter::getS( m_ParameterMap, "OPTIMIZER", "NOOPTIMIZER" );

  //============================================
  // Reading the parameters out of ranges.dat
  m_PrescaledParams = parameter::getB( m_ParameterMap, "PRESCALED_PARAMS", false );
  std::string filename = info_dir + "/ranges.dat";
  this->LoadConfigurationFile( filename );
  std::cout << "Ranges loaded" << std::endl;

  if ( !( this->m_Parameters.empty() ) ) {
    std::vector< Parameter >::const_iterator itr = this->m_Parameters.begin();
    for( itr; itr < this->m_Parameters.end(); itr++ ) {
      std::cout << itr->m_Name << std::endl;
    }
  } else {
    std::cout << "Parameters were not read in!" << std::endl;
    this->m_StateFlag = ERROR;
    return OTHER_ERROR;
  }

  std::string observables_filename = info_dir + "/pcanames.dat";
  this->LoadConfigurationFile( observables_filename );

  std::cout << "Emulated Observables Are:" << std::endl;
  if ( !( this->m_ScalarOutputNames.empty() ) ) {
    std::vector<std::string>::const_iterator itr = this->m_ScalarOutputNames.begin();
    for( itr; itr < this->m_ScalarOutputNames.end(); itr++) {
      std::cout << *itr << " ";
    }
    std::cout << std::endl;
  } else {
    std::cout << "Observables were not read in!" << std::endl;
    this->m_StateFlag=ERROR;
    return OTHER_ERROR;
  }

  //if(std::strcmp(m_Optimizer.c_str(), "MCMC" )==0 || std::strcmp(m_ModelType.c_str(), "Interpolator" ) == 0){
  this->LoadDistributions();

  std::vector< std::string > temp_logparam = parameter::getVS( m_ParameterMap, "LOG_PARAMETERS", "" );
  for ( int i = 0; i < temp_logparam.size(); i++ ) {
    if ( std::strcmp(temp_logparam[i].c_str(), "true") == 0 || std::strcmp(temp_logparam[i].c_str(), "True") == 0 ) {
      m_LogParam.push_back( true );
    } else if ( std::strcmp(temp_logparam[i].c_str(), "false") == 0 || std::strcmp(temp_logparam[i].c_str(), "False") == 0 ) {
      m_LogParam.push_back( false );
    } else {
      std::cout << "Unrecognized LogParam value " << temp_logparam[i] << std::endl;
      this->m_StateFlag=ERROR;
      return OTHER_ERROR;
    }
  }

  this->m_StateFlag=READY;
  return NO_ERROR;
}


/**
 * Loads a configuration from a file. Reads pcanames.dat or ranges.dat based on fileName
 */
MultiModel::ErrorType
MultiModel
::LoadConfigurationFile( const std::string fileName )
{
  std::fstream config_file;
  config_file.open( fileName.c_str(),std::fstream::in );
  if ( config_file ) {
    int num_inputs;
    config_file >> num_inputs;
    if ( num_inputs < 1 ) {
      std::cerr << "Number of inputs is < 1" << std::endl;
      this->m_StateFlag = ERROR;
      return OTHER_ERROR;
    }
    bool fs, ro;
    double MinR, MaxR;
    std::string name, type;

    // Check for type of file
    if ( fileName == (m_DirectoryName + "/pcanames.dat") ) {
      fs = false; //File switch to determine how the data is read in
      ro = true; //Tells the program whether to keep reading a line or not
      this->m_NumberOfOutputs = num_inputs;
      this->m_ScalarOutputNames.reserve( num_inputs );
    } else if ( fileName == (m_DirectoryName+ "/ranges.dat") ) {
      fs = true;
      ro = false;
      this->m_NumberOfParameters = num_inputs;
      this->m_Parameters.reserve(num_inputs);
    } else {
      std::cerr << "That fileName doesn't correspond to an mcmc input file" << std::endl;
      this->m_StateFlag = ERROR;
      return OTHER_ERROR;
    }
    eat_whitespace( config_file );
    int index = 0;
    while ( !config_file.eof() && index < num_inputs ) {
      discard_comments( config_file, '#' );
      if ( fs ) {//check if reading ranges
        config_file >> type;
        if ( std::strcmp(type.c_str(), "double") == 0 ) {
          ro = true;
        } else {
          ro = false;
        }
      }
      if ( ro ) {//check to see if keep reading
        config_file >> name;
        if ( index != -1 && fs ) {//check for reading ranges
          if ( std::strcmp(m_ModelType.c_str(), "Interpolator") == 0 ) {
            std::string line;
            std::getline( config_file, line );
            double r3 = sqrt( 3 );
            MinR = -r3;
            MaxR = r3;
          } else {
            if ( !m_PrescaledParams ) {
              config_file >> MinR >> MaxR;
              if ( MinR > MaxR ) {//Flip them
                double temp = MinR;
                MinR = MaxR;
                MaxR = temp;
              }
            } else {
              std::string line;
              std::getline( config_file, line );
              MinR = 0;
              MaxR = 1;
            }
          }
        }
        if ( fs ) {
          if ( index == 0 ) {
            this->m_Parameters.push_back( Parameter( name, MinR, MaxR ) );
          } else {
            if ( name.compare( this->m_Parameters.back().m_Name) != 0 ) {
              this->m_Parameters.push_back( Parameter( name, MinR, MaxR ) );
            }
          }
        } else {
          if ( index == 0 ) {
            this->m_ScalarOutputNames.push_back( name );
          } else {
            if ( name.compare( this->m_ScalarOutputNames.back() ) != 0 ) {
              this->m_ScalarOutputNames.push_back(name);
            }
          }
        }
      }
      eat_whitespace( config_file );
      index++;
    }
    if ( fs ) {
      if ( this->m_Parameters.back().m_Name.compare(0,1," ") == 0 || this->m_Parameters.back().m_Name.empty() )
        this->m_Parameters.pop_back();
      /*this->m_NumberOfParameters = this->m_Parameters.size();   // Use this for getting rid of needing to supply the number of parameters
        if ( this->m_NumberOfParameters < 1){
        std::cerr << "Number of parameters < 1" << std::endl;
        this->m_StateFlag = ERROR;
        return OTHER_ERROR;
        }*/
    } else {
      if ( this->m_ScalarOutputNames.back().compare(0,1," ") == 0 || this->m_ScalarOutputNames.back().empty() )
        this->m_ScalarOutputNames.pop_back();
      /*this->m_NumberOfOutputs = this->m_ScalarOutputNames.size();   // Use this for getting rid of the need to supply the number of outputs
        if(this->m_NumberOfOutputs < 1){
        std::cerr << "Number of scalar outputs < 1" << std::endl;
        this->m_StateFlag = ERROR;
        return OTHER_ERROR;
        }*/
    }
  } else {
    std::cout << "Could not open " << fileName.c_str() << std::endl;
    this->m_StateFlag=ERROR;
    return OTHER_ERROR;
  }
  config_file.close();

  return NO_ERROR;
}

} // end namespace madai
