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
#include <sys/stat.h>
#include <vector>

#include <cmath>
#include <map>

#include "Parameter.h"
#include "parametermap.h"

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
  Trace() {};
  Trace( const std::string info_dir,
         const std::string configuration );
  //@}
  virtual ~Trace() {}

  /** returns the number of TraceElements in this Trace. */
  unsigned int length() const;
  //@{
  /**
   * Append a \ref TraceElement with these values
   */
  void add( const std::vector< double > & parameterValues,
            const std::vector< double > & OutputValues,
            double LogLikelihood );
  void add( const std::vector< double > & parameterValues,
            const std::vector< double > & OutputValues );
  void add( const std::vector< double > & parameterValues );
  //@}
  //@{
  /**
   * Get the Nth \ref TraceElement.
   */
  TraceElement & operator[]( unsigned int idx );
  const TraceElement & operator[]( unsigned int idx ) const;
  //@}

  /**
   * print a text representation of this \c Trace.
   */
  void write( std::ostream & o ) const;

  /*
    Assert:
      FOR ALL i < this->m_TraceElements.size():
        this->m_TraceElements[i].m_ParameterValues.size() == params.size()
        this->m_TraceElements[i].m_OutputValues.size() == outputs.size()
  */
  /** \todo cleanup */
  void writeHead( std::ostream & o,
                  const std::vector< Parameter > & params ) const;
  /** \todo cleanup */
  void writeHead( std::ostream & o,
                  const std::vector< Parameter > & params,
                  const std::vector< std::string > & outputs) const;
  /** \todo cleanup */
  void PrintDataToFile( const std::vector< Parameter > & params );
  /** \todo cleanup */
  void WriteOut( const std::vector< Parameter > & params );
  /** \todo cleanup */
  void MakeTrace();

  /** \todo How does this work?  Should the Trace know about parameter
      and output names? */
  std::vector< std::string > GetParNames();

  /** \todo cleanup */
  std::string  m_TraceDirectory;
  /** \todo cleanup */
  int          m_Writeout;
  /** \todo cleanup */
  int          m_MaxIterations;
  /** \todo cleanup */
  int          m_WriteOutCounter;
  /** \todo cleanup */
  int          m_CurrentIteration;
  /** \todo cleanup */
  bool         m_AppendTrace;
  /** \todo cleanup */
  parameterMap m_TraceParameterMap;

protected:
  /** all of the trace elements */
  std::vector< TraceElement > m_TraceElements;
  /** \todo How does this work?  Should the Trace know about parameter
      and output names? Why isn't there a vector<string> m_OutputNames? */
  std::vector< std::string >  m_ParameterNames;

}; // class Trace

} // namespace madai

#endif // __Trace_h__
