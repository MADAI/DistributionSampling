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

#include "ProcessPipe.h"

#ifdef __unix__

#include <unistd.h> /* fork, pipe, dup2, close, execvp */
#include <stdio.h>  /* ANSI C standard calls (FILE*) */
#include <stdlib.h>

int perror_failure(const char *s) {
  perror( s );
  return EXIT_FAILURE;
}

/* return EXIT_FAILURE on error, EXIT_SUCCESS otherwise */
int CreateProcessPipe( ProcessPipe * pp, char * const * argv ) {
  int STANDARD_INPUT = 0;
  int STANDARD_OUTPUT = 1;
  /*int STANDARD_ERROR = 2;*/
  int question_pipe[2];
  int answer_pipe[2];
  pid_t pid;

  printf("argv[0]: %s\n", argv[0]);

  if ( argv == NULL ) {
    return perror_failure("argv is NULL :(");
  }

  if ( argv[0] == NULL ) {
    return perror_failure("argv[0] is NULL :(");
  }
	
  if ( pipe( question_pipe ) < 0 ) {
    return perror_failure("pipe [question]");
  }

  if ( pipe( answer_pipe ) < 0 ) {
    return perror_failure("pipe [answer]");
  }

  pid = fork();

  if ( pid < 0 ) {
    return perror_failure("fork");
  } else if ( pid == 0 ) {
    /* child */
    if ( -1 == close( STANDARD_INPUT ) )
      return perror_failure( "close [dpgO]" );
    if ( -1 == dup2( question_pipe[0], STANDARD_INPUT ) )
      return perror_failure( "dup2 [question/child]" );
    if ( -1 == close( STANDARD_OUTPUT ) )
      return perror_failure( "close [YOPH]" );
    if ( -1 == dup2( answer_pipe[1], STANDARD_OUTPUT ) )
      return perror_failure( "dup2 [answer/child]" );
    if ( -1 == close( question_pipe[0] ) )
      return perror_failure( "close [cJ4l]" );
    if ( -1 == close( question_pipe[1] ) )
      return perror_failure( "close [Xj78]" );
    if ( -1 == close( answer_pipe[0] ) )
      return perror_failure( "close [ttKr]" );
    if ( -1 == close( answer_pipe[1] ) )
      return perror_failure( "close [IGpw]" );
    execvp( argv[0], argv );

    /* if you reach this line, huge error. */
    fprintf( stderr, "Error while trying to execute \"%s\"\n", argv[0] );
    perror( "execvp" );
    exit( EXIT_FAILURE );
  } else {
    /* parent */
    if ( -1 == close( question_pipe[0] ) )
      return perror_failure( "close [AMLE]" );
    if ( -1 == close( answer_pipe[1] ) )
      return perror_failure( "close [6NH0]" );
    pp->question = fdopen( question_pipe[1], "w" );
    pp->answer = fdopen( answer_pipe[0], "r" );
    pp->pid = (long int)(pid); /* pid_t is probably platform specific*/

    return EXIT_SUCCESS;
  }
}

#elif defined _WIN32

#include <stdio.h>  /* fprintf() */
#include <stdlib.h> /* exit() */
#include <windows.h>
int CreateProcessPipe( ProcessPipe * pp, char * const * argv ) {
  /* FIXME */
  pp->question = NULL;
  pp->answer = NULL;
  pp->pid = -1;
  fprintf( stderr, "Not Implemented Yet :(" );
  exit( EXIT_FAILURE );
}

#else

#error "Either _WIN32 or __unix_ should be defined.\
	What kind of system is this?"

#endif
