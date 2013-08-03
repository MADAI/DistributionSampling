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

#include "ProcessPipe.h"

#if defined(__unix__) || ( defined(__APPLE__) && defined(__MACH__) )
/* to do: test this out under MacOSX */

#include <unistd.h> /* fork, pipe, dup2, close, execvp */
#include <stdio.h>  /* ANSI C standard calls (FILE*) */
#include <stdlib.h>
#include <signal.h> // kill, SIGINT

void KillProcess(ProcessPipe * pp) {
  kill((pid_t)(pp->pid), SIGINT);
}

static int perror_failure(const char *s) {
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
    exit(EXIT_FAILURE);
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

/* I learned about CreatePipe, SetHandleInformation, and CreateProcess
   from <http://support.microsoft.com/kb/190351>. What I have added is
   moving that code into a cross-platform ANSI C framework. */
#include <stdio.h>   /* ANSI C standard calls (FILE*) */
#include <io.h>
#define _WIN32_WINNT 0x501 /* enable GetProcessId() function */
#include <windows.h>
#include <fcntl.h>

void KillProcess(ProcessPipe * pp) {
  if (pp->pid != NULL)
    TerminateProcess((HANDLE)(pp->pid), 0);
}

int CreateProcessPipe( ProcessPipe * pp, char * const * argv ) {
  HANDLE OutReadHandle = NULL;
  HANDLE OutWriteHandle = NULL;
  HANDLE InReadHandle = NULL;
  HANDLE InWriteHandle = NULL;
  SECURITY_ATTRIBUTES saAttr;
  saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
  saAttr.bInheritHandle = TRUE;
  saAttr.lpSecurityDescriptor = NULL;

  if ( ! CreatePipe(&OutReadHandle, &OutWriteHandle, &saAttr, 0) ) {
    fprintf(stderr,"Error in " __FILE__ ":%d\n",__LINE__);
    return EXIT_FAILURE;
  }
  if ( ! SetHandleInformation(OutReadHandle, HANDLE_FLAG_INHERIT, 0) ) {
    fprintf(stderr,"Error in " __FILE__ ":%d\n",__LINE__);
    return EXIT_FAILURE;
  }
  if ( ! CreatePipe(&InReadHandle, &InWriteHandle, &saAttr, 0) ) {
    fprintf(stderr,"Error in " __FILE__ ":%d\n",__LINE__);
    return EXIT_FAILURE;
  }
  if ( ! SetHandleInformation(InWriteHandle, HANDLE_FLAG_INHERIT, 0) ) {
    fprintf(stderr,"Error in " __FILE__ ":%d\n",__LINE__);
    return EXIT_FAILURE;
  }
  PROCESS_INFORMATION piProcInfo;
  ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );
  STARTUPINFO siStartInfo;
  ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
  siStartInfo.cb = sizeof(STARTUPINFO);
  siStartInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
  siStartInfo.hStdOutput = OutWriteHandle;
  siStartInfo.hStdInput = InReadHandle;
  siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

  if (! CreateProcess(
    NULL,
    argv[0],        /* command line */
    NULL,           /* process security attributes */
    NULL,           /* primary thread security attributes */
    TRUE,           /* handles are inherited */
    0,              /* creation flags */
    NULL,           /* use parent's environment */
    NULL,           /* use parent's current directory */
    &siStartInfo,   /* STARTUPINFO pointer */
    &piProcInfo)) { /* receives PROCESS_INFORMATION  */
    fprintf(stderr,"Error in " __FILE__ ":%d\n",__LINE__);
    return EXIT_FAILURE;
  }

  pp->question = _fdopen(_open_osfhandle((intptr_t)(InWriteHandle), 0), "w");
  pp->answer = _fdopen(_open_osfhandle(
    (intptr_t)(OutReadHandle), _O_RDONLY), "r");
  // store the process handle, not PID.
  pp->pid = (long int)(piProcInfo.hProcess);

  CloseHandle(piProcInfo.hProcess);
  CloseHandle(piProcInfo.hThread);

  return EXIT_SUCCESS;
}

#else

#error "We only support WIN32 and POSIX.  Sorry."

#endif
