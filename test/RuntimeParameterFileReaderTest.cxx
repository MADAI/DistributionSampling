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
  std::string file_name( "/tmp/tmpRuntimeParameterFileReaderTestFile.dat" );
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
  if (runtimeParameterFileReader.GetOption("THING") != std::string("hello world")) {
    std::cerr << "failed test 1\n";
    success = false;
  }
  std::map<std::string,std::string>::const_iterator find = map.find("THING");
  if (find == map.end()) {
    std::cerr << "failed test 2\n";
    success = false;
  } else if (find->second != "hello world") {
    std::cerr << "failed test 3\n";
    success = false;
  }
  if (runtimeParameterFileReader.GetOptionAsDouble("ADOUBLE") != 1.2e3) {
    std::cerr << "failed test 4\n";
    success = false;
  }
  if (runtimeParameterFileReader.GetOptionAsInt("ITERATIONS") != 10000) {
    std::cerr << "failed test 5\n";
    success = false;
  }
  if (map.size() != 6) {
    std::cerr << "failed test 6\n";
    success = false;
  }
  if (runtimeParameterFileReader.GetOption("MODEL_OUTPUT_DIRECTORY") != "../model_output") {
    std::cerr << "failed test 7\n";
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
