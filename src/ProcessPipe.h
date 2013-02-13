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

#ifndef __process_pipe_h__
#define __process_pipe_h__

#ifndef __unix__
#define __unix__
#endif

#ifdef __cplusplus
#include <cstdio>
namespace madai {
  extern "C" {
    typedef struct process_pipe {
      std::FILE * question;
      std::FILE * answer;
      long int pid; /* in case other signals need to be sent. */
    } process_pipe;

#else /* NOT __cplusplus */

#include <stdio.h>
typedef struct process_pipe {
  FILE * question;
  FILE * answer;
  long int pid; /* in case other signals need to be sent. */
} process_pipe;

#endif /* __cplusplus */


/** returns EXIT_FAILURE on error, EXIT_SUCCESS otherwise.  Note that
 argv[0] doesn't seem to respect your PATH evironment variable, so you
 may need to pass an absolute path. argv should be NULL terminated. */
    int create_process_pipe(process_pipe * pp, char * const * argv);

#ifdef __cplusplus
  }
}
#endif /* __cplusplus */

#endif  /* __process_pipe_h__ */
