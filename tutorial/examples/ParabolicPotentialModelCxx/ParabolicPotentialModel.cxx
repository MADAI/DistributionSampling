/*=========================================================================
 *
 *  Copyright 2011-2013 The University of North Carolina at Chapel Hill
 *  and Michigan State University.  All rights reserved.
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

#include <iostream> // std::cerr
#include <fstream>  // std::ofstream
#include <cmath> // std::pow std::sqrt
#include <madai/DistributionSampling/Model.h>
#include <madai/DistributionSampling/UniformDistribution.h>
#include <madai/DistributionSampling/MetropolisHastingsSampler.h>
#include <madai/DistributionSampling/SamplerCSVWriter.h>

/**
   Parabolic Potential Model Copyright 2012-2013, Michigan State
   University.  The software was written in 2013 by Jeffrey Wyka while
   working for the MADAI project <http://madai.us/> This is inteded as
   an example of a parameterized model of a physical system with a
   parabolic potential.  */
class ParabolicPotentialModel : public madai::Model {
public:
  ParabolicPotentialModel();
  virtual ~ParabolicPotentialModel() {}
  virtual madai::Model::ErrorType GetScalarOutputsAndCovariance(
      const std::vector< double > & parameters,
      std::vector< double > & scalars,
      std::vector< double > & scalarCovariance) const;
  virtual madai::Model::ErrorType GetScalarOutputs(
      const std::vector< double > & parameters,
      std::vector< double > & scalars ) const;
};


ParabolicPotentialModel::ParabolicPotentialModel()
{
  m_StateFlag = madai::Model::UNINITIALIZED;

  // ADD INPUTS
  madai::UniformDistribution uniformDistribution;
  uniformDistribution.SetMinimum( -2.0 );
  uniformDistribution.SetMaximum( 2.0  );
  this->AddParameter( "X0", uniformDistribution );
  // This is safe because the madai::Parameter constructor clones
  // the priorDistribution.
  uniformDistribution.SetMinimum( 0.5 );
  uniformDistribution.SetMaximum( 4.0 );
  this->AddParameter( "K", uniformDistribution );
  uniformDistribution.SetMinimum( 0.5  );
  uniformDistribution.SetMaximum( 10.0 );
  this->AddParameter( "TEMP", uniformDistribution );

  // ADD OUTPUTS
  this->AddScalarOutputName("MEAN_X");
  this->AddScalarOutputName("MEAN_X_SQUARED");
  this->AddScalarOutputName("MEAN_ENERGY");
  m_GradientEstimateStepSize = 1e-9;
  m_StateFlag = madai::Model::READY;

  // ADD OBSERVATIONS
  std::vector< double > observedScalarValues(3);
  observedScalarValues[0] = 1.14;
  observedScalarValues[1] = 2.77634418605;
  observedScalarValues[2] = 3.4925;
  if (madai::Model::NO_ERROR !=
      this->SetObservedScalarValues(observedScalarValues))
    std::cerr << "Error in Model::SetObservedScalarValues()\n\n";

  // ADD OBSERVATION ERRORS
  std::vector< double > observedScalarCovariance(9,0.0);
  observedScalarCovariance[0] = 0.01;
  observedScalarCovariance[4] = 0.01;
  observedScalarCovariance[8] = 0.01;
  if (madai::Model::NO_ERROR !=
      this->SetObservedScalarCovariance(observedScalarCovariance))
    std::cerr << "Error in Model::SetObservedScalarCovariance()\n\n";
  m_StateFlag = madai::Model::READY;
}


static double GetGaussianIntegral( int power, double scale ) {
  const static double twosqrtpi = 2.0 * std::sqrt(3.14159265);
  double Integral = 1.0;
  if (power % 2 == 1) {
    Integral = 0.0;
  } else if (power % 2 == 0) {
    for (int i = power/2; i < power; ++i)
      Integral *= (i + 1);
    Integral *= twosqrtpi;
    Integral /= std::pow(2.0 * std::sqrt(scale), power + 1);
  }
  return Integral;
}


madai::Model::ErrorType
ParabolicPotentialModel::GetScalarOutputsAndCovariance(
    const std::vector< double > & parameters,
    std::vector< double > & scalars,
    std::vector< double > & scalarCovariance) const
{
  if (parameters.size() != 3)
    return madai::Model::OTHER_ERROR;
  if ((parameters[1] <= 0) || (parameters[2] <= 0)) {
    std::cerr << "Parameter cannot be non-positive\n";
    return madai::Model::OTHER_ERROR;
  }
  double x0 = parameters[0], k = parameters[1], T = parameters[2];
  // # Calculate the normalization
  // # Integrate( e^( -k/T*(x-x0)^2 ) )
  double k_over_T = k / T;
  double normalization = GetGaussianIntegral(0, k_over_T);

  double MeanX, MeanX2, MeanE, MeanX4, MeanE2;

  /*******************************************/
  // Calculate Mean X
  // Integrate( x e^( -k/T*(x - x0)^2 ) ) / normalization
  MeanX = (GetGaussianIntegral(1, k_over_T)
           + x0 * GetGaussianIntegral(0, k_over_T));
  MeanX /= normalization;
  /*******************************************/
  // Calculate Mean X^2
  // Integrate( x^2 e^( -k/T*(x - x0)^2 ) ) / normalization
  // MeanX2 = f(x0, k_over_T, normalization)
  MeanX2 = GetGaussianIntegral(2, k_over_T);
  MeanX2 += 2.0 * x0 * GetGaussianIntegral(1, k_over_T);
  MeanX2 += x0 * x0 * GetGaussianIntegral(0, k_over_T);
  MeanX2 /=normalization;

  /*******************************************/
  // Calculate Mean Energy
  // Integrate( k (x - x0)^2 * e^( -k/T*(x - x0)^2 ) ) /
  //                                   normalization + T/2
  // MeanE = f(k, T, k_over_T, normalization)
  MeanE = k* GetGaussianIntegral(2, k_over_T)/normalization + T/2.0;

  /*******************************************/
  // Calculate Mean X^4 for getting the model error
  // Integrate( x^4 e^( -k/T*(x-x0)^2 ) ) / normalization
  MeanX4 = GetGaussianIntegral(4, k_over_T);
  MeanX4 += 4.0 * x0 * GetGaussianIntegral(3, k_over_T);
  MeanX4 += 6.0*x0*x0*(GetGaussianIntegral(2, k_over_T));
  MeanX4 += 4.0*(std::pow(x0,3))*(GetGaussianIntegral(1, k_over_T));
  MeanX4 += (std::pow(x0,4))*(GetGaussianIntegral(0, k_over_T));
  MeanX4 /= normalization;

  /*******************************************/
  // Calculate Mean Energy^2 for getting the model error
  // Integrate( ( k^2*(x - x0)^4 + k*T*(x - x0)^2 )
  //                  * e^( -k/T*(x-x0)^2 ) ) + 0.25*T^2
  MeanE2 = k*k*(GetGaussianIntegral(4, k_over_T))/normalization;
  MeanE2 += k*T*(GetGaussianIntegral(2, k_over_T))/normalization;
  MeanE2 += T*T/4.0;

  /*******************************************/
  // Calculate Errors
  double ErrorX = std::sqrt(MeanX2 -std::pow( MeanX,2));
  double ErrorX2 = std::sqrt(MeanX4 - std::pow(MeanX2,2));
  double ErrorE = std::sqrt(MeanE2 - std::pow(MeanE,2));

  scalars.resize(3);
  scalars[0] = MeanX;
  scalars[1] = MeanX2;
  scalars[2] = MeanE;
  scalarCovariance.assign(9,0.0);
  scalarCovariance[0] = ErrorX * ErrorX;
  scalarCovariance[4] = ErrorX2 * ErrorX2;
  scalarCovariance[8] = ErrorE * ErrorE;
  return madai::Model::NO_ERROR;
}


madai::Model::ErrorType
ParabolicPotentialModel::GetScalarOutputs(
    const std::vector< double > & parameters,
    std::vector< double > & scalars ) const
{
  if (parameters.size() != 3)
    return madai::Model::OTHER_ERROR;
  if ((parameters[1] <= 0) || (parameters[2] <= 0)) {
    std::cerr << "Parameter cannot be non-positive\n";
    return madai::Model::OTHER_ERROR;
  }
  double x0 = parameters[0], k = parameters[1], T = parameters[2];
  // # Calculate the normalization
  // # Integrate( e^( -k/T*(x-x0)^2 ) )
  double k_over_T = k / T;
  double normalization = GetGaussianIntegral(0, k_over_T);

  double MeanX, MeanX2, MeanE;

  /*******************************************/
  // Calculate Mean X
  // Integrate( x e^( -k/T*(x - x0)^2 ) ) / normalization
  MeanX = (GetGaussianIntegral(1, k_over_T)
           + x0 * GetGaussianIntegral(0, k_over_T));
  MeanX /= normalization;

  /*******************************************/
  // Calculate Mean X^2
  // Integrate( x^2 e^( -k/T*(x - x0)^2 ) ) / normalization
  MeanX2 = GetGaussianIntegral(2, k_over_T);
  MeanX2 += 2.0 * x0 * GetGaussianIntegral(1, k_over_T);
  MeanX2 += x0 * x0 * GetGaussianIntegral(0, k_over_T);
  MeanX2 /=normalization;

  /*******************************************/
  // Calculate Mean Energy
  // Integrate( k (x - x0)^2 * e^( -k/T*(x - x0)^2 ) ) /
  //                                   normalization + T/2
  MeanE = k* GetGaussianIntegral(2, k_over_T)/normalization + T/2.0;

  /*******************************************/
  scalars.resize(3);
  scalars[0] = MeanX;
  scalars[1] = MeanX2;
  scalars[2] = MeanE;
  return madai::Model::NO_ERROR;
}


int main(int argc, char** argv) {
  static const int    MCMC_NUMBER_OF_BURN_IN_SAMPLES = 0;
  static const bool   MCMC_USE_EMULATOR_COVARIANCE   = false;
  static const double MCMC_STEP_SIZE                 = 0.1;
  if (argc < 3) {
    std::cerr << "Useage:\n    " << argv[0]
              <<" <Output_File_Name> <Number_Of_Samples>\n\n";
    return EXIT_FAILURE;
  }
  std::ofstream output(argv[1]);
  int numberOfSamples = std::atoi(argv[2]);
  ParabolicPotentialModel model;
  madai::MetropolisHastingsSampler mcmc;
  mcmc.SetStepSize( MCMC_STEP_SIZE );

  return madai::SamplerCSVWriter::GenerateSamplesAndSaveToFile(
      mcmc, model, output, numberOfSamples, MCMC_NUMBER_OF_BURN_IN_SAMPLES,
      MCMC_USE_EMULATOR_COVARIANCE, &(std::cerr));
}
