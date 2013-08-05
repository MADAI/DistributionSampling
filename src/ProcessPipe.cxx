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


#if defined(__unix__) || ( defined(__APPLE__) && defined(__MACH__))
/* to do: test this out under MacOSX */

#include <cstdio>  /* ANSI C standard calls (FILE*) */
#include <cstdlib> /* exit() EXIT_FAILURE */
#include <iostream>  /* cerr */

#include <unistd.h> /* fork, pipe, dup2, close, execvp */
#include <signal.h> // kill, SIGINT

#include "ProcessPipe.h"

struct madai::ProcessPipe::ProcessPipePrivate {
  pid_t m_Pid;
};

namespace madai {

ProcessPipe::ProcessPipe() :
  question(NULL), answer(NULL),
  m_ProcessPipeImplementation(new ProcessPipePrivate)
{
  m_ProcessPipeImplementation->m_Pid = 0;
}

bool ProcessPipe::Start(char const * const * argv) {
  if (m_ProcessPipeImplementation == NULL) {
    std::cerr << __FILE__ << ':' << __LINE__ << " Error: bad init.\n";
    return false;
  }

  if ( argv == NULL ) {
    std::cerr << __FILE__ << ':' << __LINE__ << " Error: argv is NULL.\n";
    return false;
  }

  if ( argv[0] == NULL ) {
    std::cerr << __FILE__ << ':' << __LINE__ << " Error: argv[0] is NULL.\n";
    return false;
  }

  int question_pipe[2];
  if ( pipe( question_pipe ) < 0 ) {
    perror("pipe");
    std::cerr << __FILE__ << ':' << __LINE__
              << " Error: pipe(question) failed.\n";
    return false;
  }

  int answer_pipe[2];
  if ( pipe( answer_pipe ) < 0 ) {
    perror("pipe");
    std::cerr << __FILE__ << ':' << __LINE__
              << " Error: pipe(answer) failed.\n";
  }

  pid_t pid = fork();
  if ( pid < 0 ) {
    perror("fork");
    std::cerr << __FILE__ << ':' << __LINE__
              << " Error: fork() failed.\n";
    return false;
  } else if ( pid == 0 ) {
    /* child */
    static const int STANDARD_INPUT = 0;
    static const int STANDARD_OUTPUT = 1;
    if ( -1 == close( STANDARD_INPUT ) ) {
      perror("close");
      std::cout << __FILE__ << ':' << __LINE__
                << " Error: close(STANDARD_INPUT) failed.\n";
      return false;
    }
    if ( -1 == dup2( question_pipe[0], STANDARD_INPUT ) ) {
      perror("dup2");
      std::cout << __FILE__ << ':' << __LINE__
                << " Error: dup2(STANDARD_INPUT) failed.\n";
      return false;
    }
    if ( -1 == close( STANDARD_OUTPUT ) ) {
      perror("close");
      std::cout << __FILE__ << ':' << __LINE__
                << " Error: close(STANDARD_INPUT) failed.\n";
      return false;
    }
    if ( -1 == dup2( answer_pipe[1], STANDARD_OUTPUT ) ) {
      perror("dup2");
      std::cout << __FILE__ << ':' << __LINE__
                << " Error: dup2(STANDARD_OUTPUT) failed.\n";
      return false;
    }
    if ( -1 == close( question_pipe[0] ) ) {
      perror("close");
      std::cout << __FILE__ << ':' << __LINE__
                << " Error: close(question_pipe[0]) failed.\n";
      return false;
    }
    if ( -1 == close( question_pipe[1] ) ) {
      perror("close");
      std::cout << __FILE__ << ':' << __LINE__
                << " Error: close(question_pipe[1]) failed.\n";
      return false;
    }
    if ( -1 == close( answer_pipe[0] ) ) {
      perror("close");
      std::cout << __FILE__ << ':' << __LINE__
                << " Error: close(answer_pipe[0]) failed.\n";
      return false;
    }
    if ( -1 == close( answer_pipe[1] ) ) {
      perror("close");
      std::cout << __FILE__ << ':' << __LINE__
                << " Error: close(answer_pipe[1]) failed.\n";
      return false;
    }
    execvp( argv[0], const_cast< char * const * >(argv));

    /* if you reach this line, huge error. */
    perror( "execvp" );
    std::cout << "Error while trying to execute \""<< argv[0] << "\"\n";
    std::exit(EXIT_FAILURE);
  } else {
    /* parent */
    if ( -1 == close( question_pipe[0] ) ) {
      perror("close");
      std::cout << __FILE__ << ':' << __LINE__
                << " Error: close(question_pipe[0]) failed.\n";
      return false;
    }
    if ( -1 == close( answer_pipe[1] ) ) {
      perror("close");
      std::cout << __FILE__ << ':' << __LINE__
                << " Error: close(answer_pipe[1]) failed.\n";
      return false;
    }
    question = fdopen( question_pipe[1], "w" );
    answer = fdopen( answer_pipe[0], "r" );
    m_ProcessPipeImplementation->m_Pid = pid;
    return true;
  }
}

void ProcessPipe::Stop() {
  if (question != NULL)
    std::fclose(question);
  question = NULL;
  if (answer != NULL)
    std::fclose(answer);
  answer = NULL;
  // should we pause a second for cleanup?
  if (m_ProcessPipeImplementation != NULL) {
    if (m_ProcessPipeImplementation->m_Pid > 0) {
      kill(m_ProcessPipeImplementation->m_Pid, SIGINT);
      m_ProcessPipeImplementation->m_Pid = 0;
    }
  }
}

ProcessPipe::~ProcessPipe() {
  this->Stop();
  delete m_ProcessPipeImplementation;
}
}
/******************************************************************************/
#elif defined _WIN32

#include <cstdio>  /* ANSI C standard calls (FILE*) */
#include <string> // internal function uses this for memory
#include <sstream> // internal function uses this for memory
#include <iostream> // internal function uses this for memory

#include <io.h>
#include <windows.h>
#include <fcntl.h>

#include "ProcessPipe.h"

namespace madai {
struct ProcessPipe::ProcessPipePrivate {
  HANDLE m_ProcessHandle;
};

ProcessPipe::ProcessPipe() :
  question(NULL), answer(NULL),
  m_ProcessPipeImplementation(new ProcessPipePrivate)
{
  m_ProcessPipeImplementation->m_ProcessHandle = 0;
}

ProcessPipe::~ProcessPipe() {
  this->Stop();
  delete m_ProcessPipeImplementation;
}

void ProcessPipe::Stop() {
  if (question != NULL)
    std::fclose(question);
  question = NULL;
  if (answer != NULL)
    std::fclose(answer);
  answer = NULL;
  // should we pause a second for cleanup?
  if (m_ProcessPipeImplementation != NULL) {
    if (m_ProcessPipeImplementation->m_ProcessHandle > 0) {
      TerminateProcess(m_ProcessPipeImplementation->m_ProcessHandle, 0);
      CloseHandle(m_ProcessPipeImplementation->m_ProcessHandle);
      m_ProcessPipeImplementation->m_ProcessHandle = 0;
    }
  }
}

static inline void CArglistToWin32Argstring(
    char const * const * argv,
    std::string & commandLine)
{
  std::ostringstream commandBuffer;
  while (*argv != NULL) {
    commandBuffer << '"';
    char const * s = *argv;
    while (*s != '\0') {
      if (*s == '\\') {
        int count = 1;
        do {
          ++s;
          if (*s == '\0') {
            for (int i = 0; i < count; ++i) {
              commandBuffer << '\\';
            }
            --s; // back up.
            break;
          }
          if (*s == '\\') {
            ++count;
          } else if (*s == '"') {
            for (int i = 0; i < (count * 2); ++i) {
              commandBuffer << '\\';
            }
            --s; // still need to process the quote
            // must print out odd number of backslashes
            break;
          } else {
            for (int i = 0; i < count; ++i) {
              commandBuffer << '\\';
            }
            --s;
            break;
          }
        } while (true);
      } else if (*s == '"') {
        commandBuffer << "\\\"";
      } else {
        commandBuffer << *s;
      }
      ++s;
    }
    commandBuffer << '"';
    ++argv;
    if (*argv != NULL)
      commandBuffer << ' ';
  }
  commandLine = commandBuffer.str();
}


bool ProcessPipe::Start(char const * const * argv) {
  /* I learned about CreatePipe, SetHandleInformation, and CreateProcess
     from <http://support.microsoft.com/kb/190351>. What I have added is
     moving that code into a cross-platform ANSI C framework. */
  /*
    On Windows, an args sequence is converted to a string that can be
    parsed using the following rules (which correspond to the rules
    used by the MS C runtime):

    Arguments are delimited by white space, which is either a space or
    a tab.

    A string surrounded by double quotation marks is interpreted as a
    single argument, regardless of white space contained within. A
    quoted string can be embedded in an argument.

    A double quotation mark preceded by a backslash is interpreted as
    a literal double quotation mark.

    Backslashes are interpreted literally, unless they immediately
    precede a double quotation mark.

    If backslashes immediately precede a double quotation mark, every
    pair of backslashes is interpreted as a literal backslash. If the
    number of backslashes is odd, the last backslash escapes the next
    double quotation mark as described in rule.

    Therefore, we should do a Conversion:
    -   replace '\\"' with '\\\\\"'
    -   replace '\"' with '\\\"'
    -   replace '"' with '\"'
    -   prepend and append '"'
    -   join with a ' ' in between arguments.
   */
  std::string commandLine;
  CArglistToWin32Argstring(argv, commandLine);

  HANDLE OutReadHandle = NULL;
  HANDLE OutWriteHandle = NULL;
  HANDLE InReadHandle = NULL;
  HANDLE InWriteHandle = NULL;
  SECURITY_ATTRIBUTES securityAttributes;
  securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
  securityAttributes.bInheritHandle = TRUE;
  securityAttributes.lpSecurityDescriptor = NULL;

  if ( ! CreatePipe(&OutReadHandle, &OutWriteHandle, &securityAttributes, 0) ) {
    std::cerr << "Error in " __FILE__ ":" << __LINE__ << "\n";
    return false;
  }
  if ( ! SetHandleInformation(OutReadHandle, HANDLE_FLAG_INHERIT, 0) ) {
    std::cerr << "Error in " __FILE__ ":" << __LINE__ << "\n";
    return false;
  }
  if ( ! CreatePipe(&InReadHandle, &InWriteHandle, &securityAttributes, 0) ) {
    std::cerr << "Error in " __FILE__ ":" << __LINE__ << "\n";
    return false;
  }
  if ( ! SetHandleInformation(InWriteHandle, HANDLE_FLAG_INHERIT, 0) ) {
    std::cerr << "Error in " __FILE__ ":" << __LINE__ << "\n";
    return false;
  }
  PROCESS_INFORMATION processInformation;
  ZeroMemory( &processInformation, sizeof(PROCESS_INFORMATION) );
  STARTUPINFO startupInfo;
  ZeroMemory( &startupInfo, sizeof(STARTUPINFO) );
  startupInfo.cb = sizeof(STARTUPINFO);
  startupInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
  startupInfo.hStdOutput = OutWriteHandle;
  startupInfo.hStdInput = InReadHandle;
  startupInfo.dwFlags |= STARTF_USESTDHANDLES;

  LPSTR command_line = const_cast< LPSTR >(commandLine.c_str());
  if (! CreateProcess(
    NULL,
    command_line,   /* command line */
    NULL,           /* process security attributes */
    NULL,           /* primary thread security attributes */
    TRUE,           /* handles are inherited */
    0,              /* creation flags */
    NULL,           /* use parent's environment */
    NULL,           /* use parent's current directory */
    &startupInfo,   /* STARTUPINFO pointer */
    &processInformation)) { /* receives PROCESS_INFORMATION  */
    std::cerr << "Error starting command:\n  "
              << command_line << '\n';
    // std::cerr << "Error in " __FILE__ ":" << __LINE__ << '\n';
    return false;
  }

  this->question = _fdopen(_open_osfhandle(
    (intptr_t)(InWriteHandle), 0), "w");
  this->answer = _fdopen(_open_osfhandle(
    (intptr_t)(OutReadHandle), _O_RDONLY), "r");
  m_ProcessPipeImplementation->m_ProcessHandle = processInformation.hProcess;
  CloseHandle(processInformation.hThread);
  return true;
}
}
#else

#error "We only support WIN32 and POSIX.  Sorry."

#endif
