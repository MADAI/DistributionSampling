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

#ifndef __Trace_h__
#define __Trace_h__

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <cmath>
#include <map>

#include "Parameter.h"
#include "TraceElement.h"

namespace madai {

/** \class Trace
 *
 * Traces contain all the state of the distribution sampling required to
 * replay it. */
class Trace {
public:
  //@{
  /**
   * \todo Constructor Documentation
   */
  Trace();
  //@}
  virtual ~Trace();

  /** Add a TraceElement to the Trace. */
  bool Add( const TraceElement & element );

  /** Add an entry from parameter, output values, and log-likelihood. */
  bool Add( const std::vector< double > & parameterValues,
            const std::vector< double > & outputValues,
            double logLikelihood );

  /** Add an entry from parameter and output values.
   *
   * Sets log-likelihood to 0.0. */
  bool Add( const std::vector< double > & parameterValues,
            const std::vector< double > & outputValues );

  /** Add an entry from parameter values alone.
   *
   * \todo It seems like you should always have to record an output
   * from a model. */
  bool Add( const std::vector< double > & parameterValues );

  /** Get the number of entries in the Trace. */
  unsigned int GetSize() const;

  /** Remove all elements from the Trace. */
  void Clear();

  /** Get the Nth \ref TraceElement.
   */
  //@{
  TraceElement & operator[]( unsigned int index );
  const TraceElement & operator[]( unsigned int index ) const;
  //@}

  /** Write the trace to a comma-separated value file.
   *
   * Parameters and output names are assumed to come from a
   * Model. Returns true if writing succeeded, false otherwise. */
  bool WriteCSVFile( const std::string & filename,
		     const std::vector< Parameter > & parameters,
		     const std::vector< std::string > & outputNames = std::vector< std::string >() ) const;

  void WriteCSVOutput( std::ostream & os,
                       const std::vector< Parameter > & parameters,
                       const std::vector< std::string > & outputNames = std::vector< std::string >() ) const;

  /** Import a trace from a comma-separated value file. You must tell
      it how many of the first columns are parameters and how many are
      outputs. */
  bool ImportCSVFile( const std::string & filename,
		      int numberOfParameters,
		      int numberOfOutputs );

protected:
  /** all of the trace elements */
  std::vector< TraceElement > m_TraceElements;

  void WriteHead( std::ostream & o,
                  const std::vector< Parameter > & params,
                  const std::vector< std::string > & outputs = std::vector< std::string >() ) const;

  void WriteData( std::ostream & out ) const;

}; // class Trace

} // namespace madai

#endif // __Trace_h__
