
#include <iostream>
#include <fstream>
#include <madai/DistributionSampling/Model.h>
#include <madai/DistributionSampling/MetropolisHastingsSampler.h>
#include <madai/DistributionSampling/SamplerCSVWriter.h>

class MyModel : public madai::Model {
public:
  MyModel();
  virtual ~MyModel() {}
  virtual madai::Model::ErrorType GetScalarOutputsAndCovariance(
      const std::vector< double > &, std::vector< double > &,
       std::vector< double > &) const;
  virtual madai::Model::ErrorType GetScalarOutputs(
      const std::vector< double > &, std::vector< double > &) const;
};

MyModel::MyModel() {
  // Insert your code here. This constructor should call:
  //   this->AddParameter(),
  //   this->AddScalarOutputName(),
  //   this->SetObservedScalarValues(), and
  //   this->SetObservedScalarCovariance().
}

madai::Model::ErrorType MyModel::GetScalarOutputsAndCovariance(
    const std::vector< double > & parameters,
    std::vector< double > & outputs,
    std::vector< double > & outputCovariance) const {
  (void)parameters; (void)outputs; (void)outputCovariance;
  // Insert your code here.
  return madai::Model::NO_ERROR;
}

madai::Model::ErrorType MyModel::GetScalarOutputs(
    const std::vector< double > & parameters,
    std::vector< double > & outputs ) const {
  (void)parameters; (void)outputs;
  // Insert your code here.
  return madai::Model::NO_ERROR;
}

int main(int, char**) {
  MyModel model;
  madai::MetropolisHastingsSampler mcmc;
  mcmc.SetStepSize(0.1);
  std::ofstream output("output.csv");
  int NumberOfSamples = 1000000;
  int NumberOfBurnInSamples = 0;
  bool UseEmulatorCovariance = true;

  return madai::SamplerCSVWriter::GenerateSamplesAndSaveToFile(
      mcmc, model, output, NumberOfSamples, NumberOfBurnInSamples,
      UseEmulatorCovariance, &(std::cerr));
}
