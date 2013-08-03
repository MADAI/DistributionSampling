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

#ifndef madai_ProcessPipe_h_included
#define madai_ProcessPipe_h_included

#ifdef __cplusplus
#include <cstdio>
namespace madai {
  extern "C" {
    typedef struct ProcessPipe {
      std::FILE * question;
      std::FILE * answer;
      long int pid; /* in case other signals need to be sent. */
    } ProcessPipe;

#else /* NOT __cplusplus */

#include <stdio.h>
/** Container for process-related information */
typedef struct ProcessPipe {
  /** stdin for the process */
  FILE * question;

  /** stdout for the process */
  FILE * answer;

  /** Process id kept in case other signals need to be sent. */
  long int pid;

} ProcessPipe;

#endif /* __cplusplus */


/** returns EXIT_FAILURE on error, EXIT_SUCCESS otherwise.  Note that
 argv[0] doesn't seem to respect your PATH evironment variable, so you
 may need to pass an absolute path. argv should be NULL terminated. */
 int CreateProcessPipe(ProcessPipe * pp, char * const * argv);

 /**
    OS-dependent way of killing a process */
 void KillProcess(ProcessPipe * pp);

#ifdef __cplusplus
  }
}
#endif /* __cplusplus */

#endif  /* madai_ProcessPipe_h_included */
