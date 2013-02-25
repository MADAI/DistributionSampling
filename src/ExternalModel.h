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
  typedef enum {
    NO_COVARIANCE,
    TRIANGULAR_COVARIANCE,
    FULL_MATRIX_COVARIANCE,
    DIAGONAL_MATRIX_COVARIANCE, /* VARIANCE only */
  } CovarianceMode;

  ExternalModel();
  virtual ~ExternalModel();

  /**
   * Loads a configuration from a file.  The format of the file is
   * defined by this function.  We'll lock it down later.
   */
  virtual ErrorType LoadConfigurationFile( const std::string fileName ) {
    return METHOD_NOT_IMPLEMENTED;
  }

  /**
   * Start the external process and leave it open for queries.
   */
  virtual ErrorType StartProcess( const std::string & processPath,
                                  const std::vector< std::string > & arguments );

  /**
   * Stop the external process.
   */
  virtual ErrorType StopProcess();

  /**
   * Get the scalar outputs from the model evaluated at x.  If an
   * error happens, the scalar output array will be left incomplete.
   */
  virtual ErrorType GetScalarOutputsAndCovariance(
      const std::vector< double > & parameters,
      std::vector< double > & scalars,
      std::vector< double > & scalarCovariance) const;

  /**
   * Get the scalar outputs from the model evaluated at x.
   */
  virtual ErrorType GetScalarOutputs( const std::vector< double > & parameters,
                                      std::vector< double > & scalars ) const;

  /**
   * METHOD_NOT_IMPLEMENTED
   */
  virtual Model::ErrorType GetLikeAndPrior(
    const std::vector<double>&, double&, double&) const {
    return METHOD_NOT_IMPLEMENTED;
  }


private:
  ProcessPipe m_Process;
  CovarianceMode m_CovarianceMode;
}; // end ExternalModel

} // end namespace madai

#endif // __ExternalModel_h__
