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

/*
 * GaussianProcessEmulator Class
 *
 * \author Hal Canary <cs.unc.edu/~hal/>
 * \author Christopher Coleman-Smith <cec24@phy.duke.edu>
 * \author Cory Quammen <cs.unc.edu/~cquammen>
 */

#ifndef madai_GaussianProcessEmulator_h_included
#define madai_GaussianProcessEmulator_h_included

#include <iostream> // std::istream std::ostream
#include <vector>   // std::vector
#include <Eigen/Dense> // Eigen::MatrixXd, Eigen::VectorXd
#include "Parameter.h" // madai::Parameter

namespace madai {
/**
 *  \class GaussianProcessEmulator
 *
 * This class is an implementation of a Gaussian Process Emulator that
 * emulates a potentially high-dimensional function, producing
 * function values and (optionally) uncertainties at arbitrary points
 * in the domain of the function. The emulator is trained with a set
 * of training points sampled from the function domain via, for
 * example, a LatinHypercubeGenerator.
 *
 * In addition to the training points, the emulator is defined by
 * weighting hyperparameters that are determined by a training
 * process. These hyperparameters influence a covariance function that
 * specifies the covariance of the outputs. Currently, training
 * involves setting each of the weights proportional to the absolute
 * difference of the interquartile range. */
class GaussianProcessEmulator
{


public:
  // ENUMERATIONS
  /**
    CovarianceFunction describes which type of covariance function our
    GaussianProcessEmulator uses.*/
  typedef enum {
    /** 
\f$ \Vert{}u-v\Vert{}_\Theta{} = \sqrt{\sum_{i=0}^{P-1} \frac{(u_i - v_i)^2}{(\theta{}_{2+i})^2}} \qquad
 \delta{}_{uv} = 1 \text{ if } (\Vert{}u-v\Vert{}_\Theta{} <
 \epsilon{}) \textrm{ else } \delta{}_{uv} = 0 \f$

\f$ c_\Theta{}(u, v) = \theta{}_0 \exp\left({-\tfrac{1}{2}
 \Vert{}u-v\Vert{}_\Theta{}^2}\right) + \theta{}_1\delta{}_{uv} \f$
     */
    SQUARE_EXPONENTIAL_FUNCTION,

    /**
\f$  \Vert{}u-v\Vert{}_\Theta{} = \sqrt{\sum_{i=0}^{P-1} \frac{(u_i - v_i)^2}{(\theta{}_{3+i})^2}} \qquad
 \delta{}_{uv} = 1 \text{ if } (\Vert{}u-v\Vert{}_\Theta{} <
 \epsilon{}) \textrm{ else } \delta{}_{uv} = 0 \f$

\f$ c_\Theta{}(u, v) = \theta{}_0 \exp\left({-\tfrac{1}{2}
 \Vert{}u-v\Vert{}_\Theta{}^{\theta{}_2}}\right) +
 \theta{}_1\delta{}_{uv}  \f$
     */
    POWER_EXPONENTIAL_FUNCTION,

    /**
\f$ \Vert{}u-v\Vert{}_\Theta{} = \sqrt{\sum_{i=0}^{P-1} \frac{(u_i - v_i)^2}{(\theta{}_{2+i})^2}} \qquad
 \delta{}_{uv} = 1 \text{ if } (\Vert{}u-v\Vert{}_\Theta{} <
 \epsilon{}) \textrm{ else } \delta{}_{uv} = 0 \f$

\f$ c_\Theta{}(u, v) = {\theta{}_0} \left({ 1 + \sqrt{3} \Vert{}u-v\Vert{}_\Theta{} }\right)
\exp \left({ - \sqrt{3} \Vert{}u-v\Vert{}_\Theta{} }\right) +
 \theta{}_1\delta{}_{uv} \f$
     */
    MATERN_32_FUNCTION,

    /**
\f$ \Vert{}u-v\Vert{}_\Theta{} = \sqrt{\sum_{i=0}^{P-1} \frac{(u_i - v_i)^2}{(\theta{}_{2+i})^2}} \qquad
 \delta{}_{uv} = 1 \text{ if } (\Vert{}u-v\Vert{}_\Theta{} < \epsilon{}) \textrm{ else } \delta{}_{uv} = 0 \f$

\f$ c_\Theta{}(u, v) = {\theta{}_0} \left({ 1 + \sqrt{5} \Vert{}u-v\Vert{}_\Theta{}
 + \tfrac{5}{3} \left({\Vert{}u-v\Vert{}_\Theta{}}\right)^2  }\right) + \theta{}_1\delta{}_{uv} \f$ */
    MATERN_52_FUNCTION,

    /** Unknown covariance function. It should be an error if this is
     *  ever the setting value. */
    UNKNOWN_FUNCTION
  } CovarianceFunctionType;

  /**
    Enumerator encoding the GaussianProcessEmulator's status. */
  typedef enum {
    /** Ready to emulate values at points in the parameter space. */
    READY,

    /** Emulator internal variables are not cached. */
    UNCACHED,

    /** The emulator has not yet been trained. */
    UNTRAINED,

    /** The emulator has not been initialized. */
    UNINITIALIZED,

    /** A significant error has occurred. */
    ERROR
  } StatusType;

  // METHODS
  /**
   * Default constructor which makes an uninitialized
   * GaussianProcessEmulator.
   */
  GaussianProcessEmulator();

  /**
   * This takes a GaussianProcessEmulator with training data and
   * decomposes the training data into its principal components.
   *
   * \returns true on success. */
  bool PrincipalComponentDecompose();

  /**
   * Retains only the eigenvectors and eigenvalues necessary for the
   * desired fraction resolving power.
   *
   * \param fractionResolvingPower Valid range is [0, 1].
   * \return True on success
   */
  bool RetainPrincipalComponents(double fractionResolvingPower);

  /**
   * This takes sets default values for all of the
   * hyperparameters of the GaussianProcessModel.
   *
   * \returns True on success.
   */
  bool BasicTraining(
    CovarianceFunctionType covarianceFunction,
    int regressionOrder,
    double defaultNugget,
    double amplitude,
    double scale);

  /**
   * This performs a more thorough training process on the
   * GausianProcessEmulator.
   *
   * \returns true on sucess.
   * \warning Not implemented yet.
   */
  bool Train(
    CovarianceFunctionType covarianceFunction,
    int regressionOrder);

  /**
   * Execute the model at an input point x.  Save a lot of time by not
   * calculating the covariance error.
   *
   * \param x Point in parameter space where emulator should be
   * evaluated.
   * \param y Function value at x.
   */
  bool GetEmulatorOutputs (
    const std::vector< double > & x,
    std::vector< double > & y) const;

  /**
   * Execute the model at an input point x.
   *
   * The covariance returned will be a matrix flattened to fit within
   * the output vector.
   * \param x Point in parameter space where emulator should be
   * evaluated.
   * \param y Function value at x.
   * \param ycov Covariance at point x.
   */
  bool GetEmulatorOutputsAndCovariance (
    const std::vector< double > & x,
    std::vector< double > & y,
    std::vector< double > & ycov) const;

  /**
   * Get the gradients of the model outputs at x.
   *
   * \param x Point in parameter space where emulator should be
   * evaluated.
   * \param gradients Partial gradients of the emulator output at x.
   */
  bool GetGradientOfEmulatorOutputs(
    const std::vector< double > & x,
    std::vector< double > & gradients ) const;

  /**
   * Get the gradients of the elements of the output covaiance matrix.
   *
   * \param x Point in parameter space where emulator should be
   * evaluated.
   * \param gradients Partial gradients of the emulator covariance at x.
   */
  bool GetGradientsOfCovariances(
    const std::vector< double > & x,
    std::vector< Eigen::MatrixXd > & gradients) const;

  /**
   * Check status of the emulator.
   *
   * \return One of the emulator StatusTypes.
   */
  StatusType CheckStatus();

  /**
   * Check status of the emulator.
   *
   * \return One of the emulator StatusTypes.
   */
  StatusType GetStatus() const;

  /**
   *
   *  \return Status as a string.
   */
  std::string GetStatusAsString() const;

  /**
   * These values are used for scaling the model output prior to
   * principal component analysis.  They are the sum of the training
   * output variances squared and the observed variances squared.
   *
   * \param x Storage for the uncertainty scales.
   * \return False if uncertainty scales could not be computed, true otherwise.
   */
  bool GetUncertaintyScales(std::vector< double > & x) const;

  /**
   * Get the observed experimental values
   *
   * \param x Storage for the observed values.
   * \return Always returns true.
   */
  bool GetObservedValues(std::vector< double > & x);

  /**
   * Get the uncertainty scales as a diagonal matrix.
   *
   * \param x Storage for the full diagonal matrix as a flattened
   * array.
   */
  bool GetUncertaintyScalesAsCovariance(std::vector< double > & x) const;

  /**
   * Use the uncertainty scales, the training values, the mean value
   * of all the training outputs, the retained eigenvectors from the
   * principal component analysis to determine the z values for each
   * single variable model.
   */
  bool BuildZVectors();

  /**
   * Once Load(), Train(), or BasicTraining() finishes, calculate and
   * cache some data to make calling GetEmulatorOutputsAndCovariance()
   * faster.
   */
  bool MakeCache();

  /**
   * Current status of the GaussianProcessEmulator.
   */
  StatusType m_Status;

  /**
   * Human-readable comments about the GaussianProcessEmulator.
   */
  std::vector<std::string> m_Comments;

  /**
   * A list of input parameters.
   */
  std::vector<madai::Parameter> m_Parameters;

  /**
   * A list of output names.
   */
  std::vector<std::string> m_OutputNames;

  /**
   * Number of input parameters to the model.
   */
  int m_NumberParameters;

  /**
   * Number of  model outputs.
   */
  int m_NumberOutputs;

  /**
   * Number of training points.
   */
  int m_NumberTrainingPoints;

  /**
   * Number of principle components in the output space. This is the
   * number of SingleModels the GaussianProcessEmulator has.
   */
  int m_NumberPCAOutputs;

  /**
   * Original training parameter values: the "design"
   * (m_NumberTrainingPoints rows by m_NumberParameters columns)
   */
  Eigen::MatrixXd m_TrainingParameterValues;

  /**
   * Original training output values.
   * (m_NumberTrainingPoints rows by m_NumberOutputs columns)
   */
  Eigen::MatrixXd m_TrainingOutputValues;

  /**
   * The mean values of the columns of outputValues
   * (size == m_NumberOutputs)
   */
  Eigen::VectorXd m_TrainingOutputMeans;

  /**
   * The mean variance of the model outputs.
   */
  Eigen::VectorXd m_TrainingOutputVarianceMeans;

  /**
   * These values are not used by the emulator, but represent the
   * experimentally observed mean values of the output variables.
   * They are stored in this class for convenience.
   */
  //@{
  Eigen::VectorXd m_ObservedValues;
  Eigen::VectorXd m_ObservedVariances;
  //@}

  /**
   * These values are used for scaling the model output prior to
   * principal component analysis. They are the sum of the training
   * output variances squared and the observed variances squared.
   */
  Eigen::VectorXd m_UncertaintyScales;

  //@{
  /**
   * The eigenvalues and vectors from the PCA decomposition of
   * outputValues.  Only the first numberPCAOutputs of them are kept.
   * Consequently, PCAEigenvalues is a vector of length
   * numberPCAOutputs and PCAEigenvectors is a matrix of size
   * numberPCAOutputs-by-numberOutputs.
   */
  Eigen::VectorXd m_RetainedPCAEigenvalues;
  Eigen::MatrixXd m_RetainedPCAEigenvectors;
  Eigen::VectorXd m_PCAEigenvalues;
  Eigen::MatrixXd m_PCAEigenvectors;
  //@}

protected:

  /**
   * Use m_TrainingOutputVarianceMeans and m_ObservedUncertainty to
   *  compute the output uncertainty scales.
   */
  bool BuildUncertaintyScales();

public:

  /**
   * This represents a single PCA-decomposed model.
   */
  struct SingleModel {
    SingleModel();

    /**
     * Calulate the covariance between two points in parameter space
     * using m_Thetas and m_CovarianceFunction.
     *
     * \param v1 One point in parameter space.
     * \param v2 Another point in parameter space.
     * \return The covariance between the points.
     */
    double CovarianceCalc(
        const Eigen::VectorXd & v1,
        const Eigen::VectorXd & v2) const;

    /**
     * Get the gradient of the covariance function between two points
     * in parametespace using m_Thetas and m_CovarianceFunction.
     *
     * \param v1 One point in parameter space.
     * \param v2 Another point in parameter space.
     * \param gradient Storage for the computed gradient.
     * \return False if there was a problem computing the gradient of
     *         the covariance calculation, true otherwise.
     */
    bool GetGradientOfCovarianceCalc(
        const Eigen::VectorXd & v1,
        const Eigen::VectorXd & v2,
        Eigen::VectorXd & gradient) const;

    /**
     * Called by GaussianProcessEmulator::MakeCache().  Given a set of
     * (thetas, covarianceFunction, regressionOrder), populate
     * m_CInverse, m_BetaVector, m_GammaVector, m_RegressionMatrix1,
     * m_RegressionMatrix2, and m_HMatrix.
     */
    bool MakeCache();

    /**
     * Sets default values for all of the hyperparameters.
     *
     * \return True on success.
     */
    bool BasicTraining(
        CovarianceFunctionType covarianceFunction,
        int regressionOrder,
        double defaultNugget,
        double amplitude,
        double scale);

    /**
     * Trains the submodel.
     *
     * \param covarianceFunction The type of covariance function.
     * \param regressionOrder Order of polynomial for regression prior
     * to PCA.
     * \return True on success.
     * \warning Not implemented.
     */
    bool Train(
        CovarianceFunctionType covarianceFunction,
        int regressionOrder);

    /**
     * Execute the model at an input point x and get the mean and
     * variance of the output.
     */
    bool GetEmulatorOutputsAndCovariance(
        const std::vector< double > & x,
        double & mean,
        double & variance) const;

    /**
     * Execute the model at an input point x and get the output mean.
     * Save a lot of time by not calculating the error.
     */
    bool GetEmulatorOutputs(
        const std::vector< double > & x,
        double & mean) const;

    /**
     * Get the gradient of the model outputs at an input point x.
     */
    bool GetGradientOfEmulatorOutputs(
        const std::vector< double > & x,
        std::vector< double > & gradient ) const;

    /**
     * Get the gradient of the variance at an input point x.
     */
    bool GetGradientOfCovariance(
        const std::vector< double > & x,
        std::vector< double > & gradient ) const;

    /**
     * A pointer to the parent.
     */
    const GaussianProcessEmulator * m_Parent;

    /**
     * Which covariance function is used?
     */
    CovarianceFunctionType m_CovarianceFunction;

    /**
     * Which order polynomial is used as a regression function?
     */
    int m_RegressionOrder;

    /**
     * zMatrix = (retained PCA eigenvectors)^T
     * (outputValues_standardized)/ Each SingleModel gets one column
     * (of length numberTrainingPoints) from the zMatrix.  This is the
     * vector zValues.
     */
    Eigen::VectorXd m_ZValues;

    /**
     * These are the hyperparameters.
     */
    Eigen::VectorXd m_Thetas;

    //@{
    /**
     * Things we cached; values to carry from one calulation to the next.
     */
    Eigen::MatrixXd m_CInverse;

    // [NxN] m_CInverse = CMatrix.inverse();
    Eigen::MatrixXd m_RegressionMatrix1;

    // [FxF] m_RegressionMatrix1
    //     = (HMatrix.transpose() * m_CInverse * HMatrix).inverse();
    Eigen::MatrixXd m_RegressionMatrix2;

    // [FxN] m_RegressionMatrix2
    //     = (m_CInverse * HMatrix).transpose();
    Eigen::VectorXd m_BetaVector;

    //  [F]  m_BetaVector
    //   = m_RegressionMatrix * HMatrix.transpose() * m_CInverse * m_ZValues;
    Eigen::VectorXd m_GammaVector;

    //  [N]  m_GammaVector
    //        = m_CInverse * (m_ZValues - (HMatrix * m_BetaVector));
    //@}
  };

  /**
   * An array of length numberPCAOutputs/
   */
  std::vector< SingleModel > m_PCADecomposedModels;
};

} // end namespace madai


#endif /* madai_GaussianProcessEmulator_h_included */
