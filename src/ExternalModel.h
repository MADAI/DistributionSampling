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

#ifndef madai_ExternalModel_h_included
#define madai_ExternalModel_h_included

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
  /** Encodes covariance type */
  typedef enum {
    /** No covariance. */
    NO_COVARIANCE,

    /** Only the non-redundant half of the covariance matrix is
     *  specified as an upper triangular matrix. */
    TRIANGULAR_COVARIANCE,

    /** The covariance is specified as a full matrix. */
    FULL_MATRIX_COVARIANCE,

    /** The covariance is specified as a diagonal matrix (variance
    only). */
    DIAGONAL_MATRIX_COVARIANCE
  } CovarianceMode;

  ExternalModel();
  virtual ~ExternalModel();

  /** Start the external process and leave it open for queries
   *
   * \param processPath Path to the process to be run.
   * \param arguments Vector of arguments to pass to the process. */
  virtual ErrorType StartProcess( const std::string & processPath,
                                  const std::vector< std::string > & arguments );

  /** Stop the external process */
  virtual ErrorType StopProcess();

  virtual ErrorType GetScalarOutputsAndCovariance(
      const std::vector< double > & parameters,
      std::vector< double > & scalars,
      std::vector< double > & scalarCovariance) const;

  /** Get the scalar outputs from the model evaluated at a position in
   * the parameter space. */
  virtual ErrorType GetScalarOutputs( const std::vector< double > & parameters,
                                      std::vector< double > & scalars ) const;

private:
  /** Container for process-related information */
  ProcessPipe m_Process;

  /** Shape of the covariance matrix reported by the external model */
  CovarianceMode m_CovarianceMode;

}; // end ExternalModel

} // end namespace madai

#endif // madai_ExternalModel_h_included
