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

#include <cstdio> // std::FILE
namespace madai {

/**
   \todo document this.
 */
class ProcessPipe {
public:
  std::FILE * question;
  std::FILE * answer;
  ProcessPipe();

  /**
     \return true on success, false otherwise.
     If an error occurs, will print debugging info to stderr. */
  bool Start(char const * const * argv);
  // pointer to const pointer to const data.

  /**
     Will attempt to stop the running process. Will set question and
     answer to NULL. */
  void Stop();

  ~ProcessPipe();

private:
  /**
     Explicitly disallowed */
  ProcessPipe& operator=(const madai::ProcessPipe &);
  /**
     Explicitly disallowed */
  ProcessPipe(const madai::ProcessPipe &);

  struct ProcessPipePrivate;
  /**
     Opaque Pointer for OS-specific Private Implementation. */
  ProcessPipePrivate * m_ProcessPipeImplementation;
};
}
#endif  /* madai_ProcessPipe_h_included */
