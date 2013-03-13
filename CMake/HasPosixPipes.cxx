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

#include <cstdio>

// Try to include headers we need
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


int main( int argc, char* argv[] )
{
  int pipeInfo[2];

  pipe( pipeInfo );

  fork();

  close( 0 );

  dup2( pipeInfo[0], 0 );

  fdopen( pipeInfo[0], "w" );

  return EXIT_SUCCESS;
}
