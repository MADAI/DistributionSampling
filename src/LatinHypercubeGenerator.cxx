/*=========================================================================
 *
 *  Copyright 2011-2013 The University of North Carolina at Chapel Hill
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

#include "LatinHypercubeGenerator.h"

#include "Random.h"


namespace madai {

LatinHypercubeGenerator
::LatinHypercubeGenerator() :
  m_Random( new Random() )
{
}


LatinHypercubeGenerator
::~LatinHypercubeGenerator()
{
  delete m_Random;
}


void
LatinHypercubeGenerator
::SetStandardDeviations( double standardDeviations )
{
  m_StandardDeviations = standardDeviations;
}


double
LatinHypercubeGenerator
::GetStandardDeviations() const
{
  return m_StandardDeviations;
}


void
LatinHypercubeGenerator
::SetDivideSpaceByPercentile( bool value )
{
  m_DivideSpaceByPercentile = value;
}


bool
LatinHypercubeGenerator
::GetDivideSpaceByPercentile() const
{
  return m_DivideSpaceByPercentile;
}


std::vector< Sample >
LatinHypercubeGenerator
::Generate( int numberOfParameters,
            int numberOfTrainingPoints,
            const std::vector< double > & parameterMinima,
            const std::vector< double > & parameterMaxima )
{
  // Generate one vector per parameter with numberOfTrainingPoints
  // subdivisions in the range [ parameterMinima, parameterMaxima ]
  std::vector< std::vector< double > > parameterSubdivisions( numberOfParameters );
  for ( int i = 0; i < numberOfParameters; ++i ) {
    double rangeOverN = (parameterMaxima[i] - parameterMinima[i]) /
      static_cast< double >( numberOfTrainingPoints );
    double start = (0.5 * rangeOverN ) + parameterMinima[i];
    parameterSubdivisions[i].resize( numberOfTrainingPoints );
    for ( int j = 0; j < numberOfTrainingPoints; ++j ) {
      parameterSubdivisions[i][j] = (rangeOverN * j) + start;
    }

    // Randomly shuffle the subdivisions
    m_Random->ShuffleVector( parameterSubdivisions[i] );
  }

  // Now loop over the samples and create a parameter position from
  // the shuffled subdivisions.
  std::vector< Sample > samples( numberOfTrainingPoints );
  for ( int i = 0; i < numberOfTrainingPoints; ++i ) {
    std::vector< double > parameterValues( numberOfParameters );
    for ( int j = 0; j < numberOfParameters; ++j ) {
      parameterValues[j] = parameterSubdivisions[j][i];
    }

    samples[i].m_ParameterValues = parameterValues;
  }

  return samples;
}


} // end namespace madai
