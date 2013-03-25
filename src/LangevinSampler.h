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

#ifndef __LangevinSampler_h__
#define __LangevinSampler_h__

#include "Sampler.h"


namespace madai {
  
/**
 * \class LangevinSampler
 *
 * This is an implementation of a langevin search algorithm.
 * It considers the gradient of the likelihood at a point, and
 * moves according to the langevin equation.
 */

class LangevinSampler : public Sampler {
public:
  // Constructor
  LangevinSampler();
  // Destructor
  virtual ~LangevinSampler();
  
  virtual Sample NextSample();
  
  virtual void SetTimeStep( double TimeStep );
  virtual double GetTimeStep() { return this->m_TimeStep; }
  
  virtual void SetKickStrength( double Impulse );
  virtual double GetKickStrength() { return this->m_KickStrength; }
  
  virtual void SetMeanTimeBetweenKicks( double MeanTime );
  virtual double GetMeanTimeBetweenKicks() { return this->m_MeanTime; }
  
  virtual void SetDragCoefficient( double DragCoefficient );
  virtual double GetDragCoefficient() { return this->m_DragCoefficient; }
  
  virtual void SetMassScale( double MassScale );
  virtual double GetMassScale() { return this->m_MassScale; }
  
  virtual ErrorType SetVelocity( const std::string & parameterName, double Velocity );
  virtual std::vector< double > GetVelocities() { return this->m_CurrentVelocities; }
  
protected:
  
  double m_TimeStep;
  double m_MeanTime;
  double m_KickStrength;
  double m_TimeBeforeNextKick;
  double m_DragCoefficient;
  double m_MassScale;
  
  // Keep track of the velocities
  std::vector< double > m_CurrentVelocities;
  
  virtual void Initialize( const Model * model );

private:

  // Random number object
  madai::Random m_Random;
  
}; // end class LangevinSampler

} // end namespace madai

#endif // __LangevinSampler_h__
