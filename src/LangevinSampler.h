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

#ifndef madai_LangevinSampler_h_included
#define madai_LangevinSampler_h_included

#include "Sampler.h"


namespace madai {

/**
 * \class LangevinSampler
 *
 * This is an implementation of a Langevin search algorithm.
 * It considers the gradient of the likelihood at a point, and
 * moves according to the Langevin equation.
 */

class LangevinSampler : public Sampler {
public:
  // Constructor
  LangevinSampler();
  // Destructor
  virtual ~LangevinSampler();

  /**
   * Compute the next set of parameters, output scalar values, and
   * log likelihood
   *
   * \return A new Sample. */
  virtual Sample NextSample();

  /**
   * Set/get the timestep. */
  //@{
  virtual void SetTimeStep( double TimeStep );
  virtual double GetTimeStep() { return this->m_TimeStep; }
  //@}

  /**
   * Set/get the kick strength. */
  //@{
  virtual void SetKickStrength( double Impulse );
  virtual double GetKickStrength() { return this->m_KickStrength; }
  //@}

  /**
   * Set/get the mean time between kicks. */
  //@{
  virtual void SetMeanTimeBetweenKicks( double MeanTime );
  virtual double GetMeanTimeBetweenKicks() { return this->m_MeanTime; }
  //@}

  /**
   * Set/get the drag coefficient. */
  //@{
  virtual void SetDragCoefficient( double DragCoefficient );
  virtual double GetDragCoefficient() { return this->m_DragCoefficient; }
  //@}

  /**
   * Set/get the mass scale. */
  //@{
  virtual void SetMassScale( double MassScale );
  virtual double GetMassScale() { return this->m_MassScale; }
  //@}

  /**
   * Set/get the velocity scale. */
  //@{
  virtual ErrorType SetVelocity( const std::string & parameterName, double Velocity );
  virtual std::vector< double > GetVelocities() { return m_CurrentVelocities; }
  //@}

protected:
  /** Timestep parameter. */
  double m_TimeStep;

  /** Mean time parameter. */
  double m_MeanTime;

  /** Kick strength parameter. */
  double m_KickStrength;

  /** Time before next kick parameter. */
  double m_TimeBeforeNextKick;

  /** Drag coefficient parameter. */
  double m_DragCoefficient;

  /** Mass scale parameter. */
  double m_MassScale;

  /** Keeps track of the velocities. */
  std::vector< double > m_CurrentVelocities;

  /** Initialize this sampler with the given Model. */
  virtual void Initialize( const Model * model );

private:

  /** Instance of a random number generator. */
  madai::Random m_Random;

}; // end class LangevinSampler

} // end namespace madai

#endif // madai_LangevinSampler_h_included
