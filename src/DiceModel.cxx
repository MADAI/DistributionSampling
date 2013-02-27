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

#include "DiceModel.h"


namespace madai {


DiceModel
::DiceModel()
{
  this->m_StateFlag = UNINITIALIZED;
}


DiceModel
::DiceModel( const std::string info_dir )
{
  this->m_StateFlag = UNINITIALIZED;
  this->LoadConfigurationFile(info_dir);
}


Model
::ErrorType
DiceModel
::LoadConfigurationFile( const std::string info_dir )
{
  m_DirectoryName = info_dir;
  m_ConfigFile    = m_DirectoryName + "/DiceConfig.param";
  parameter::ReadParsFromFile( m_ParameterMap, m_ConfigFile.c_str() );
  m_Sum                = parameter::getB( m_ParameterMap, "SUM_DICE", true );
  m_Distinguishable    = parameter::getB( m_ParameterMap, "DISTINGUISHABLE", false );
  m_NumberOfParameters = parameter::getI( m_ParameterMap, "DICE", 5 );
  m_Sides              = parameter::getI( m_ParameterMap, "SIDES", 6 );
  
  // Weigted Dice Possibility
  m_Weighted           = parameter::getB( m_ParameterMap, "WEIGHTED", false );
  for( unsigned int j = 0; j < m_NumberOfParameters; j++ ) {
    std::stringstream ss;
    std::string wname = "WEIGHTS";
    ss << j+1;
    wname += ss.str();
    m_Weights.push_back( parameter::getV( m_ParameterMap, wname.c_str(), "0" ) );
    ss.str( string() );
  }
  for( unsigned int k = 0; k < m_NumberOfParameters; k++) {
    double temp = 0;
    for( unsigned int l = 0; l < m_Sides; l++ ) {
      temp += m_Weights[k][l];
    }
    m_TotalWeights.push_back( temp );
  }
  
  // Factor for calculating the likelihood and add parameters
  m_Denom = 1;
  std::string par_name;
  for ( unsigned int i = 0; i < m_NumberOfParameters; i++ ) {
    std::stringstream ss;
    ss << i;
    std::string addon = ss.str();
    m_Denom *= m_Sides;
    this->AddParameter( par_name+addon, 0.5, ( double( m_Sides ) + 0.5 ) );
    this->AddScalarOutputName( par_name+addon+"-Output" );
  }
  std::cerr << m_Denom << std::endl;
  
  this->m_StateFlag = READY;
  
  return Model::NO_ERROR;
}


Model
::ErrorType
DiceModel
::GetScalarOutputs( const std::vector< double > & parameters,
                    std::vector< double > & scalars ) const
{
  scalars.clear();
  for ( unsigned int i = 0; i < parameters.size(); i++ ) {
    double temp;
    if ( parameters[i] < 0.5 || parameters[i] > ( double( m_Sides ) + 0.5 ) ) {
      std::cerr << i << "th parameter out of range" << std::endl;
      return OTHER_ERROR;
    }
    for( unsigned int j = 0; j < m_Sides; j++ ) {
      if( parameters[i] <= ( double( j ) + 1.5) ) {
        temp = double( j ) + 1.0;
        break;
      }
    }
    scalars.push_back( temp );
  }
  
  return NO_ERROR;
}


double
DiceModel
::GetWeightedDiceLikelihood( const std::vector< double > & outputs ) const
{
  double Like;
  if ( m_Distinguishable ) {
    Like = 1.0;
    for ( unsigned int i = 0; i < outputs.size(); i++ ) {
      Like *= ( double( m_Weights[i][int( outputs[i] - 1 )] ) / double( m_TotalWeights[i] ) );
    }
  } else {
    // First, bin or sum
    int* bins = new int[m_Sides]();
    double dsum = 0;
    for ( unsigned int j = 0; j < outputs.size(); j++ ) {
      if ( m_Sum ) {
        dsum += outputs[j];
      } else {
        for ( unsigned int k = 0; k < m_Sides; k++ ) {
          if ( outputs[j] == double(k + 1) ) {
            bins[k]++;
            break;
          }
        }
      }
    }
    Like = 0.0;
    int* counter = new int[outputs.size()]();
    // all counters set to 0 ( die value 1 )
    bool done = false;
    //double Sum = 0;
    while ( !done ) {
      bool AddLike = false;
      if ( m_Sum ) {
        // Get sum of counters to compare to outputs
        double temp_sum = 0;
        for ( unsigned int j = 0; j < outputs.size(); j++ ) {
          temp_sum += double( counter[j] + 1 );
        }
        if ( temp_sum == dsum ) { // Check if sums are the same
          AddLike = true;
        }
      } else {
        int* temp_bins = new int[m_Sides](); 
        for ( unsigned int j = 0; j < outputs.size(); j++ ) {
          for ( unsigned int k = 0; k < m_Sides; k++ ) {
            if ( counter[j] == k ) {
              temp_bins[k]++;
              break;
            }
          }
        }
        // Compare binning of outputs and counters
        int bcomp = 0;
        for ( unsigned int j = 0; j < m_Sides; j++ ) {
          if ( bins[j] == temp_bins[j] ) {
            bcomp++;
          } else {
            break;
          }
        }
        if ( bcomp == m_Sides ) {
          AddLike = true;
        }
      }
      if ( AddLike ) {
        // Get Probability for current configuration
        double temp = 1.0;
        for ( unsigned int i = 0; i < outputs.size(); i++ ) {
          temp *= ( double( m_Weights[i][counter[i]] ) / double( m_TotalWeights[i] ) );
        }
        // Add probability of current config to likelihood
        Like += temp;
      }
      // Check to see if done going through configurations
      int dcounter = 0;
      bool* at_end = new bool[outputs.size()]();
      for ( unsigned int j = 0; j < outputs.size(); j++ ) {
        if ( counter[j] == (m_Sides - 1) ) {
          at_end[j] = true;
          dcounter++;
        } else {
          at_end[j] = false;
        }
      }
      if ( dcounter == outputs.size() ) {
        done = true;
      } else { // Move to next combination
        for ( unsigned int j = 0; j < outputs.size(); j++ ) {
          if ( !at_end[outputs.size() - j - 1] ) {
            counter[outputs.size() - j - 1]++;
            for ( unsigned int k = 0; k < j; k++ ) {
              counter[outputs.size() - k - 1] = 0;
            }
            break;
          }
        }
      }
    }
  }
  return Like;
}


double
DiceModel
::GetRegularDiceLikelihood( const std::vector< double > & outputs ) const
{
  double Like;
  if ( m_Sum ) {
    Like = 0.0;
    int sum = 0;
    for ( unsigned int l = 0; l < outputs.size(); l++) {
      sum += outputs[l];
    }
    for ( unsigned int iter = 0; iter <= floor((sum-outputs.size())/double(m_Sides)); iter++ ) {
      double temp = 0;
      int iter2;
      temp += outputs.size();
      if ( iter % 2 == 1 ) {
        temp *= -1;
      }
      // (k-m_Sides*iter-1)!
      for ( iter2 = 0; iter2 < (sum-m_Sides*iter-1); iter2++ ){
        temp *= ( iter2 + 1 );
      }
      // 1/(k-m_Sides*iter-n)!
      for ( iter2 = 0; iter2 < (sum-m_Sides*iter-outputs.size()); iter2++ ) {
        temp /= double( iter2 + 1 );
      }
      // 1/(n-iter)!
      for ( iter2 = 0; iter2 < (outputs.size()-iter); iter2++ ) {
        temp /= double( iter2 + 1 );
      }
      // 1/iter!
      for ( iter2 = 0; iter2 < iter; iter2++ ) {
        temp /= double( iter2 + 1 );
      }
      Like += temp;
    }
    Like /= double( m_Denom );
  } else if ( !m_Distinguishable ) {
    // Find total number of possible permutations of the dice
    double TotalPerms = 1.0;
    for ( unsigned int m = 0; m < outputs.size(); m++ ) {
      TotalPerms *= (m + 1);
    }
    // Bin the dice values
    int* bins = new int[m_Sides]();
    for ( unsigned int j = 0; j < outputs.size(); j++ ) {
      for ( unsigned int k = 0; k < m_Sides; k++ ) {
        if ( outputs[j] == double(k + 1) ) {
          bins[k]++;
          break;
        }
      }
    }
    // Eliminate repeated permutations
    double factor = 1.0;
    for ( unsigned int n = 0; n < m_Sides; n++ ) {
      if ( bins[n] > 1 ) {
        for ( unsigned int index = 0; index < bins[n]; index++ ) {
          factor *= double(index + 1);
        }
      }
    }
    TotalPerms /= factor;
    Like = TotalPerms / double( m_Denom );
  } else {
    Like = 1.0 / double( m_Denom );
  }
  return Like;
}


Model
::ErrorType
DiceModel
::GetLikeAndPrior( const std::vector< double > & parameters,
                   double & Like,
                   double & Prior) const
{
  // Get outputs
  std::vector< double > outputs;
  this->GetScalarOutputs( parameters, outputs );
  
  if ( m_Weighted ) {
    Like = GetWeightedDiceLikelihood( outputs );
  } else {
    Like = GetRegularDiceLikelihood( outputs );
  }
  Prior = 1.0;
  Like = log( Like );
  
  return Model::NO_ERROR;
}

} // end namespace madai
