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

#include "GaussianDistribution.h"
#include "Random.h"
#include "UniformDistribution.h"


namespace madai {

LatinHypercubeGenerator
::LatinHypercubeGenerator() :
  m_Random( new Random() ),
  m_StandardDeviations( 3.0 ),
  m_PartitionSpaceByPercentile( false )
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
::SetPartitionSpaceByPercentile( bool value )
{
  m_PartitionSpaceByPercentile = value;
}


bool
LatinHypercubeGenerator
::GetPartitionSpaceByPercentile() const
{
  return m_PartitionSpaceByPercentile;
}


void
LatinHypercubeGenerator
::PartitionDimension( int numberOfTrainingPoints,
                      const Parameter & parameter,
                      std::vector< double > & subdivisions )
{
  subdivisions.resize( numberOfTrainingPoints );

  const Distribution * priorDistribution = parameter.GetPriorDistribution();

  double rangeOverN = 1.0 / static_cast< double >( numberOfTrainingPoints );
  if ( m_PartitionSpaceByPercentile ) {
    for ( int i = 0; i < numberOfTrainingPoints; ++i ) {
      double percentile = rangeOverN * ( i + 0.5 );
      subdivisions[i] = priorDistribution->GetPercentile( percentile );
    }
  } else {
    // Divide space evenly
    const UniformDistribution * uniformDistribution =
      dynamic_cast< const UniformDistribution * >( priorDistribution );
    const GaussianDistribution * gaussianDistribution =
      dynamic_cast< const GaussianDistribution * >( priorDistribution );

    double parameterMinimum = 0.0, parameterMaximum = 1.0;
    if ( uniformDistribution ) {
      parameterMinimum = uniformDistribution->GetMinimum();
      parameterMaximum = uniformDistribution->GetMaximum();
    } else if ( gaussianDistribution ) {
      double span = m_StandardDeviations * gaussianDistribution->GetStandardDeviation();
      parameterMinimum = gaussianDistribution->GetMean() - span;
      parameterMaximum = gaussianDistribution->GetMean() + span;
    }

    rangeOverN *= ( parameterMaximum - parameterMinimum );
    double start = ( 0.5 * rangeOverN ) + parameterMinimum;
    for ( int i = 0; i < numberOfTrainingPoints; ++i ) {
      subdivisions[i] = rangeOverN * i + start;
    }
  }
}


std::vector< Sample >
LatinHypercubeGenerator
::Generate( int numberOfTrainingPoints,
            const std::vector< Parameter > parameters )
{
  // Generate one vector per parameter with numberOfTrainingPoints
  // subdivisions in the range [ parameterMinima, parameterMaxima ]
  std::vector< std::vector< double > > parameterSubdivisions( parameters.size() );
  for ( size_t i = 0; i < parameters.size(); ++i ) {
    this->PartitionDimension( numberOfTrainingPoints, parameters[i],
                              parameterSubdivisions[i] );

    // Randomly shuffle the subdivisions
    m_Random->ShuffleVector( parameterSubdivisions[i] );
  }

  // Now loop over the samples and create a parameter position from
  // the shuffled subdivisions.
  std::vector< Sample > samples( numberOfTrainingPoints );
  for ( int i = 0; i < numberOfTrainingPoints; ++i ) {
    std::vector< double > parameterValues( parameters.size() );
    for ( size_t j = 0; j < parameters.size(); ++j ) {
      parameterValues[j] = parameterSubdivisions[j][i];
    }

    samples[i].m_ParameterValues = parameterValues;
  }

  return samples;
}


} // end namespace madai
