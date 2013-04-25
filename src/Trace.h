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

#ifndef madai_Trace_h_included
#define madai_Trace_h_included

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <cmath>
#include <map>

#include "Parameter.h"
#include "Sample.h"

namespace madai {

/** \class Trace
 *
 * Traces contain a record of the samples drawn by a Sampler from a
 * Model. */
class Trace {
public:
  /** Default constructor that creates an empty trace. */
  Trace();
  virtual ~Trace();

  /** Add a Sample to the Trace.
   *
   * \param sample The Sample to add to the Trace.
   * \return True if the sample was added to the Trace, false
   * otherwise. */
  bool Add( const Sample & sample );

  /** Add an entry from parameter, output values, and log-likelihood. */
  bool Add( const std::vector< double > & parameterValues,
            const std::vector< double > & outputValues,
            double logLikelihood );

  /** Add an entry from parameter and output values.
   *
   * Sets log-likelihood to 0.0.
   *
   * \param parameterValues List of parameter values.
   * \param outputValues List of output values produced by a Model
   *   evaluated at the parameter values.
   * \return True if the sample was added to the Trace, false
   * otherwise. */
  bool Add( const std::vector< double > & parameterValues,
            const std::vector< double > & outputValues );

  /** Add an entry from parameter values alone.
   *
   * \param parameterValues List of parameter values.
   * \return True if the sample was added to the Trace, false
   * otherwise. */
  bool Add( const std::vector< double > & parameterValues );

  /** Get the number of entries in the Trace.
   *
   * \return The number of entries in the Trace. */
  unsigned int GetSize() const;

  /** Remove all elements from the Trace. */
  void Clear();

  /** Get the n-th Sample.
   */
  //@{
  Sample & operator[]( unsigned int index );
  const Sample & operator[]( unsigned int index ) const;
  //@}

  /** Write the trace to a comma-separated value file.
   *
   * Parameters and output names are assumed to come from a
   * Model.
   *
   * \param filename Name of the CSV file to write.
   * \param parameters List of Parameters. Should be obtained from a Model.
   * \param outputNames Name of the outputs. Shoulde be obtained from a Model.
   * \return Returns true if writing succeeded, false otherwise. */
  bool WriteCSVFile( const std::string & filename,
         const std::vector< Parameter > & parameters,
         const std::vector< std::string > & outputNames = std::vector< std::string >() ) const;

  /** Write the CSV-formatted output to a std::ostream.
   *
   * \param os Output stream.
   * \param parameters List of Parameters. Should be obtained from a Model.
   * \param outputNames Name of the outputs. Shoulde be obtained from
   * a Model. */
  void WriteCSVOutput( std::ostream & os,
                       const std::vector< Parameter > & parameters,
                       const std::vector< std::string > & outputNames = std::vector< std::string >() ) const;

  /** Import a trace from a comma-separated value file
   *
   *  You must tell it how many of the first columns are parameters
   * and how many are outputs as this information is not saved in the
   * file.
   *
   * \param filename Name of the CSV file to import.
   * \param numberOfParameters How many of the columns should be
   * considered parameter columns.
   * \param numberOfOutputs How many of the columns should be
   * considered output columns.
   * \return True if the import succeeded, false otherwise. */
  bool ImportCSVFile( const std::string & filename,
                      int numberOfParameters,
                      int numberOfOutputs );

protected:
  /** List of Samples in the trace. */
  std::vector< Sample > m_Samples;

  /** Writes a comma-delimited header to an output stream.
   *
   * \param o The output stream.
   * \param parameters A vector of parameters from a Model.
   * \param outputs A vector of Model output names. */
  void WriteHead( std::ostream & o,
                  const std::vector< Parameter > & parameters,
                  const std::vector< std::string > & outputs = std::vector< std::string >() ) const;

  /** Writes comma-delimited Sample parameters, outputs, and
   * log-likelihoods to an output stream.
   *
   * \param out The output stream. */
  void WriteData( std::ostream & out ) const;

}; // class Trace

} // namespace madai

#endif // madai_Trace_h_included
