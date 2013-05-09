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

#include <iostream>
#include <fstream>

#include <madaisys/SystemTools.hxx> // madaisys::RemoveFile
#include "RuntimeParameterFileReader.h"

/**
   \todo document */
int main(int, char**) {
  std::string file_name( "tmpRuntimeParameterFileReaderTestFile.dat" );
  std::ofstream testFile;
  testFile.open(file_name.c_str());
  testFile <<
    "#comment  \n"
    "  MODEL_OUTPUT_DIRECTORY   ../model_output  # Comment # comment\n"
    "EXPERIMENTAL_RESULTS_DIRECTORY ../experimental_results\n"
    "ITERATIONS 10000\n"
    "   #another comment  \n"
    "ADOUBLE 1.2e3 #another comment##\n"
    "USE_EMULATED_ERROR false\n"
    "THING           \t\thello     world  #another comment \n"
    "\n"
    "\n"
    "\n";
  testFile.close();
  madai::RuntimeParameterFileReader runtimeParameterFileReader;
  runtimeParameterFileReader.ParseFile(file_name);
  const std::map<std::string, std::string> map
    = runtimeParameterFileReader.GetAllOptions();

  bool success = true;
  std::string expectedString( "hello     world" );
  std::string retrievedString( runtimeParameterFileReader.GetOption("THING") );
  if ( retrievedString != expectedString ) {
    std::cerr << "Expected option THING to have value '" << expectedString
              << "' but got '" << retrievedString << "'\n";
    success = false;
  }

  std::map<std::string,std::string>::const_iterator find = map.find("THING");
  if (find == map.end()) {
    std::cerr << "Could not find option 'THING' in map returned from "
              << "RuntimeParameterFileReader\n";
    success = false;
  } else if (find->second != expectedString) {
    std::cerr << "Option 'THING' was expected to have value '" << expectedString
              << "' but got '" << find->second << "'\n";
    success = false;
  }

  double expectedDouble = 1.2e3;
  double retrievedDouble = runtimeParameterFileReader.GetOptionAsDouble("ADOUBLE");
  if ( retrievedDouble != expectedDouble ) {
    std::cerr << "Option 'ADOUBLE' was expected to have value " << expectedDouble
              << ", but got " << retrievedDouble << "\n";
    success = false;
  }

  int expectedInt = 10000;
  int retrievedInt = runtimeParameterFileReader.GetOptionAsInt("ITERATIONS");
  if ( retrievedInt != expectedInt ) {
    std::cerr << "Option 'ITERATIONS' was expected to have value " << expectedInt
              << ", but got " << retrievedInt << "\n";
    success = false;
  }
  if (map.size() != 6) {
    std::cerr << "There should be 6 options, but only " << map.size()
              << " were in the map.\n";
    success = false;
  }

  expectedString = "../model_output";
  retrievedString = runtimeParameterFileReader.GetOption("MODEL_OUTPUT_DIRECTORY");
  if ( retrievedString != expectedString ) {
    std::cerr << "Option 'MODEL_OUTPUT_DIRECTORY' was expected to have value '"
              << expectedString << "' but got '" << find->second << "'\n";

    std::cerr << "Mapped values are: \n";
    for ( std::map< std::string, std::string >::const_iterator iter = map.begin();
          iter != map.end(); ++iter ) {
      std::cerr << "name: '" << iter->first << "', value: '" << iter->second
                << "'\n";
    }
    success = false;
  }

  madaisys::SystemTools::RemoveFile(file_name.c_str());
  if (success) {
    std::cerr << "RuntimeParameterFileReaderTest: passed all tests.\n";
    return EXIT_SUCCESS;
  } else {
    return EXIT_FAILURE;
  }
}
