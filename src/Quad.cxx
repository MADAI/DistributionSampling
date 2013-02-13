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

#include "Quad.h"

#include "Model.h"


namespace madai {


QuadHandler
::QuadHandler(parameterMap *parmap, Model * in_Model) :
  m_Model( in_Model ),
  m_Path( NULL )
{
  // cout << "QuadHandler: Constructor Start" << endl;

  m_QuadScriptHome = parameter::getS( *parmap, "QuadFILEPATH", "./" );
  //Observables = parameter::getS(mcmc->parmap, "OBSERVABLES", "Observables not specified");
  //Observables = mcmc->Observables;

  // cout << "The Quad is located at " << QuadScriptHome << endl;
  m_EmOutputFile = m_QuadScriptHome + "/src/QuadOutput.txt";

  std::fstream f;

  //create input/output files. I think this works, but it seems crude to me (fix later)
  f.open( m_EmOutputFile.c_str(), ios::out );
  f.flush();
  f.close();

  std::stringstream ss;
  std::vector< Parameter > const * parameters = &(m_Model->GetParameters());
  for ( int i = 0; i < parameters->size(); i++ )
    ss << (*parameters)[i].m_Name << " ";
  ss >> m_EmulatedParams;

  // cout << "QuadHandler: Constructor Done." << endl;
}


QuadHandler
::~QuadHandler()
{
  if ( remove( m_EmOutputFile.c_str() ) != 0 ) {
    std::cerr << "Warning: ~QuadHandler: Unable to erase input/output/error files." << std::endl;
  }

  //cd back to original directory.
  std::string command = "cd " + std::string( m_Path );
  int temp = system( command.c_str() );
}


void
QuadHandler
::QueryQuad( std::vector< double > Theta, std::vector< double > &Means, std::vector< double > &Errors )
{
  // cout << "Querying Quad." << endl;
  std::ofstream outputfile;
  std::ifstream inputfile;
  std::string command;
  std::string currentline;
  char * token;
  int NumDataRows = 1;

  //m_EmulatedParams = mcmc->EmulatorParams;

  command = m_QuadScriptHome + "/src/quad ";
  for ( int i = 0; i < Theta.size(); i++ ) {
    std::stringstream ss;
    ss << Theta[i];
    command = command + ss.str() + " ";
  }
  command = command + "> "+ m_EmOutputFile;

  int result = system( command.c_str() );

  if ( result != 0 ) {
    std::cerr << "Error: Unable to run Quad." << std::endl;
    exit( 1 );
  }

  inputfile.open( m_EmOutputFile.c_str() );
  if ( inputfile ) {
    while( !inputfile.eof() ) {
      std::getline( inputfile, currentline, '\n' );
      // std::cout << currentline << std::endl;
      if ( currentline.compare(0, 1, "#" ) != 0 && !currentline.empty() ) { //Comments have a # character
        if ( currentline.compare(0, 5, "Error" ) == 0 ) { //pass Quad errors to cerr stream
          std::cerr << "Error during emulation: Check " << m_EmOutputFile << std::endl;
          std::cerr << currentline << std::endl;
          exit( 1 );
        }

        std::stringstream ss;

        double tempnum;

        ss << currentline;
        while( ss >> tempnum ) {
          Means.push_back( tempnum );
          //Errors.push_back(1); //The errors are always 1
        }
        NumDataRows++;
      }
    }
    if ( Means.size() == 17 ) {
      float myints[] = {23.2096,28.6014,45.2516,62.9723,0.390475,0.36656,0.383316,10.566,28.7311,45.3162,62.7765,0.00482302,0.00455732,0.00534056,0.326125,0.31816,0.326674};
      Errors.assign( myints, myints+17 );
    }
    if ( Means.size() == 6 ) {
      float myints[] = {1,1,1,1,1,1};
      Errors.assign( myints, myints+6 );
    }

    inputfile.close();
  } else {
    std::cerr << "Unable to open Quad output file." << std::endl;
    exit( 1 );
  }
  if ( Means.size() != Errors.size() ) {
    std::cerr << "Error: Quad output size mismatch. Error in reading Quad output in." << std::endl;
    std::cerr << "Mean vector size: " << Means.size() << " Error vector size: " << Errors.size() << std::endl;
    exit( 1 );
  }

}

} // end namespace madai
