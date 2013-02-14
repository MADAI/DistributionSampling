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

#ifndef __ExternalModel_h__
#define __ExternalModel_h__

#include "Model.h"

#include "ProcessPipe.h"
/* madai::ProcessPipe struct, madai::CreateProcessPipe fn.
 Note that ProcessPipe is full of POSIX-specific inter-process
 communication code.  We can and should fix that at some point.  Until we
 fix that, this class will only compile for a POSIX (Unix, Linux,
 BSD, MacOS X) target.  */

#include <iostream>
#include <string>
#include <vector>


namespace madai {

/** \class ExternalModel
 *
 * Specifies an interface to external executables that take
 * parameters and produce model outputs. */
class ExternalModel : public Model {
public:
  
  ExternalModel();
  virtual ~ExternalModel();

  /** 
   * Loads a configuration from a file.  The format of the file is
   * defined by this function.  We'll lock it down later.
   */
  virtual ErrorType LoadConfigurationFile( const std::string fileName );

  /**
   * Start the external process and leave it open for queries.
   */
  virtual ErrorType StartProcess( const std::string & processPath,
                                  const std::vector< std::string > & arguments );

  /** 
   * Get the scalar outputs from the model evaluated at x.  If an
   * error happens, the scalar output array will be left incomplete.
   */
  virtual ErrorType GetScalarOutputs( const std::vector< double > & parameters,
                                      std::vector< double > & scalars ) const;

  // Not implemented yet.
  // Proposed function for interaction with the MCMC
  virtual ErrorType GetLikeAndPrior( const std::vector< double > & parameters,
                                     double & Like,
                                     double & Prior ) const;

private:
  std::vector< std::string > m_CommandArguments;

  ProcessPipe m_Process;
}; // end Model

} // end namespace madai

#endif // __ExternalModel_h__
