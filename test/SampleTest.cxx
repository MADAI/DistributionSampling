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

#include <cstdlib>
#include <iostream>

#include "TraceElement.h"

int main( int argc, char* argv[] ) {

  // By default, TraceElements should be invalide
  madai::TraceElement element;
  if ( element.IsValid() ) {
    std::cerr << "TraceElement's should not be valid when first created." << std::endl;
    return EXIT_FAILURE;
  }

  element.m_ParameterValues.push_back( 1.0 );

  if ( !element.IsValid() ) {
    std::cerr << "TraceElement should be valid when one or more parameters is set."
	      << std::endl;
    return EXIT_FAILURE;
  }

  element.Reset();

  if ( element.IsValid() ) {
    std::cerr << "TraceElement should not be valid after being reset." << std::endl;
    return EXIT_FAILURE;
  }

  element.m_OutputValues.push_back( 0.0 );

  if ( !element.IsValid() ) {
    std::cerr << "TraceElement should be valid when one or more outputs is set." << std::endl;
    return EXIT_FAILURE;
  }

  element.Reset();

  element.m_Comments.push_back( "Comment" );

  if ( !element.IsValid() ) {
    std::cerr << "TraceElement should be valid when one or more comments is set." << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
