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

#include "GaussianProcessEmulatorTestGenerator.h"

#include "LatinHypercubeGenerator.h"
#include "Random.h"
#include "Sample.h"


GaussianProcessEmulatorTestGenerator
::GaussianProcessEmulatorTestGenerator(
    void (*model)(const std::vector< double > &, std::vector< double > &),
    int numberParameters,
    int numberOutputs,
    int numberTrainingPoints,
    const std::vector< double > & parameterMinima,
    const std::vector< double > & parameterMaxima) :
  m_NumberParameters(numberParameters),
  m_NumberOutputs(numberOutputs),
  m_NumberTrainingPoints(numberTrainingPoints),
  m_ParameterMinima(parameterMinima),
  m_ParameterMaxima(parameterMaxima)
{
  assert (parameterMinima.size() >= numberParameters);
  assert (parameterMaxima.size() >= numberParameters);
  assert((numberTrainingPoints > 0) && (numberParameters > 0));

  madai::LatinHypercubeGenerator latinHypercubeGenerator;
  std::vector< madai::Sample > samples =
    latinHypercubeGenerator.Generate( numberParameters, numberTrainingPoints,
                                      parameterMinima, parameterMaxima );

  m_X.resize(numberTrainingPoints,numberParameters);
  m_Y.resize(numberTrainingPoints,numberOutputs);

  std::vector< double > x(numberParameters+4,0.0);
  std::vector< double > y(numberOutputs+4,0.0);

  for(int i = 0; i < numberTrainingPoints; ++i) {
    for(int j = 0; j < numberParameters; ++j) {
      m_X(i,j) = samples[i].m_ParameterValues[j];
      x[j] = m_X(i,j);
    }
    (*model)(x,y);
    for(int j = 0; j < numberOutputs; ++j) {
      m_Y(i,j) = y[j];
    }
  }
}

void
GaussianProcessEmulatorTestGenerator
::WriteTrainingFile(std::ostream & o)
{
  o.precision(17);
  o << "#\n#\n#\nVERSION 1\nPARAMETERS\n"<< m_NumberParameters << "\n";
  for(int i = 0; i < m_NumberParameters; ++i)
    o << "param_" << i << " UNIFORM " << m_ParameterMinima[i]
      << " " << m_ParameterMaxima[i] << "\n";
  o << "OUTPUTS\n" << m_NumberOutputs << "\n";
  for(int i = 0; i < m_NumberOutputs; ++i)
    o << "output_" << i << "\n";
  o << "NUMBER_OF_TRAINING_POINTS\n" << m_NumberTrainingPoints << "\n";
  o << "PARAMETER_VALUES\n" << m_NumberTrainingPoints
    << " " << m_NumberParameters << "\n" << m_X << "\n";
  o << "OUTPUT_VALUES\n" << m_NumberTrainingPoints
    << " " << m_NumberOutputs << "\n" << m_Y << "\nEND_OF_FILE\n";
  o.flush();
}
