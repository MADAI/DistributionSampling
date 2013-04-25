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

#ifndef madai_GaussianProcessEmulatorTestGenerator_h_included
#define madai_GaussianProcessEmulatorTestGenerator_h_included

#include <iostream>
#include <vector>
#include <Eigen/Dense>

#include <Parameter.h>

class GaussianProcessEmulatorTestGenerator {
public:
  GaussianProcessEmulatorTestGenerator(
    void (*model)(const std::vector< double > & , std::vector< double > & ),
    int numberParameters,
    int numberOutputs,
    int numberTrainingPoints,
    const std::vector< madai::Parameter > & parameters );

  void WriteTrainingFile(std::ostream & o);

  static void LatinHypercube(
      int numberParameters,
      int numberTrainingPoints,
      double * columnMajorMatrix,
      const double * parameterMinima,
      const double * parameterMaxima);

  Eigen::MatrixXd m_X;
  Eigen::MatrixXd m_Y;
  int m_NumberParameters;
  int m_NumberOutputs;
  int m_NumberTrainingPoints;
  std::vector< madai::Parameter > m_Parameters;
};

#endif /* madai_GaussianProcessEmulatorTestGenerator_h_included */
