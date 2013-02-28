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

#include "MarkovChainMonteCarloSampler.h"


namespace madai {


MarkovChainMonteCarloSampler
::MarkovChainMonteCarloSampler( const Model *in_model,
           const std::string info_dir ) :
  Sampler( in_model )
{
  // First check to see if the model supplies a method for
  // calculating the Likelihood and Prior.
  if ( !this->IsLikeAndPrior() ) {
    std::cerr << "GetLikeAndPrior() needs to be defined in order to use the MCMC" << std::endl;
    exit( 1 );
  }

  m_DirectoryName = info_dir;
  std::string ParFileName = info_dir + "/defaultpars/mcmc.param";

  parameter::ReadParsFromFile( m_LocalParameterMap, ParFileName.c_str() );

  m_BurnIn        = parameter::getI( m_LocalParameterMap, "BURN_IN", 0 );
  m_RandomTheta0  = parameter::getB( m_LocalParameterMap, "RANDOM_THETA0", false );
  m_VizTrace      = parameter::getB( m_LocalParameterMap, "VISUALIZE_TRACE", true );
  m_Quiet         = parameter::getB( m_LocalParameterMap, "QUIET", false );
  m_AppendTrace   = parameter::getB( m_LocalParameterMap, "APPEND_TRACE", false );
  m_RescaledTrace = parameter::getB( m_LocalParameterMap, "RESCALED_TRACE", false );
  m_CreateTrace   = parameter::getB( m_LocalParameterMap, "CREATE_TRACE", true );
  m_LogProposal   = parameter::getB( m_LocalParameterMap, "LOGPROPOSAL", true );
  m_LogPrior      = parameter::getB( m_LocalParameterMap, "LOGPRIOR", true );
  m_LogLike       = parameter::getB( m_LocalParameterMap, "LOGLIKE", true );
  m_Debug         = parameter::getB( m_LocalParameterMap, "DEBUG", false );

  // Define how to take and evaluate steps
  this->LoadStepParameters();

  m_RandomNumber = new CRandom( 1234 );

  // Get initial theta
  if ( m_RandomTheta0 ) {
    if ( m_Debug ) {
      std::cerr << "Debug mode is for MCMC Core testing. Random initial theta and debug mode are incompatible" << std::endl;
      exit( 1 );
    }
    m_InitialTheta = this->GetRandomTheta0( time( NULL ) );
  } else {
    m_InitialTheta = this->GetTheta0FromFile();
  }
  m_CurrentParameters = m_InitialTheta;
  // Get Likelihood, Prior and Scale for the initial theta
  m_Model->GetLikeAndPrior( m_CurrentParameters, m_LikelihoodCurrent, m_PriorCurrent );
  m_ScaleCurrent = (rand() / double(RAND_MAX));
}

MarkovChainMonteCarloSampler
::~MarkovChainMonteCarloSampler()
{
}

void
MarkovChainMonteCarloSampler
::NextSample( Trace *ThetaOutsList )
{
  double alpha, LOGBF;
  if ( m_LogLike ) {
    LOGBF = 0;
  } else {
    LOGBF = 1;
  }
  std::vector< double > Temp_Theta;
  m_ScaleNew = (rand() / double(RAND_MAX));

  if ( m_IterationNumber == 1) {
    m_BestLikelihood=m_LikelihoodCurrent;
    m_BestParameterSet = m_InitialTheta;
    if ( m_CreateTrace ) {
      m_VizCount = parameter::getI( m_LocalParameterMap, "VIZ_COUNT", floor(ThetaOutsList->m_MaxIterations/200) );
    }
  }

  Temp_Theta = this->TakeStep( m_CurrentParameters, m_ScaleNew );
  m_Model->GetLikeAndPrior( Temp_Theta, m_LikelihoodNew, m_PriorNew );
  m_ProposalNew = this->EvaluateProposal( m_CurrentParameters, Temp_Theta, m_ScaleCurrent ); // m_ProposalNew
  m_ProposalCurrent = this->EvaluateProposal( Temp_Theta, m_CurrentParameters, m_ScaleNew ); // m_ProposalCurrent

  if ( m_LogLike ) {
    LOGBF += m_LikelihoodNew - m_LikelihoodCurrent;
    LOGBF += log( m_PriorNew ) - log(m_PriorCurrent);
    LOGBF += log( m_ProposalNew ) - log(m_ProposalCurrent);
    if ( !m_Quiet ) {
      printf( " LogLikeNew=%g   LogLikeCurrent=%g\n", m_LikelihoodNew, m_LikelihoodCurrent );
    }
  } else {
    LOGBF *= exp( m_LikelihoodNew ) / exp( m_LikelihoodCurrent );
    LOGBF *= m_PriorNew / m_PriorCurrent;
    LOGBF *= m_ProposalNew / m_ProposalCurrent;
    if ( !m_Quiet ) {
      printf( " LikeNew=%g   LikeCurrent=%g\n", exp( m_LikelihoodNew ), exp( m_LikelihoodCurrent ) );
    }
  }
  if ( !m_Quiet ) {
    if ( m_LogPrior ) {
      printf( " LogPriorNew=%g   LogPriorCurrent=%g\n", log( m_PriorNew ), log( m_PriorCurrent ) );
    } else {
      printf( " PriorNew=%g   PriorCurrent=%g\n", m_PriorNew, m_PriorCurrent );
    }
    if ( m_LogProposal ) {
      printf( " LogProposalNew=%g   LogProposalCurrent=%g\n", log( m_ProposalNew ), log( m_ProposalCurrent ) );
    } else {
      printf( " ProposalNew=%g   ProposalCurrent=%g\n", m_ProposalNew, m_ProposalCurrent );
    }
  }

  if ( m_LogLike ) {
    alpha = min( 1.0, exp( LOGBF ) );
  } else {
    alpha = min( 1.0, LOGBF );
  }

  if ( !m_Quiet ) {
    printf( "%5d\talpha=%6.5f\t", m_IterationNumber, alpha);
  }

  double r;
  if ( m_Debug ) {
    r = rand() / double(RAND_MAX);
  } else {
    r = this->m_RandomNumber->ran();
  }

  if ( alpha > r ) { //Accept the proposed set.
    if ( !m_Quiet ) {
      printf("Accept\n");
    }
    if ( m_IterationNumber > m_BurnIn ) {
      m_AcceptCount++;
    }
    m_LikelihoodCurrent = m_LikelihoodNew;
    m_PriorCurrent      = m_PriorNew;
    m_CurrentParameters = Temp_Theta;
    m_ScaleCurrent      = m_ScaleNew;
    if ( m_LikelihoodCurrent > m_BestLikelihood && m_IterationNumber > 1 ) {
      m_BestLikelihood   = m_LikelihoodNew;
      m_BestParameterSet = m_CurrentParameters;
      if ( !m_Quiet ) {
        printf("XXXXXXXXX YIPPEE!! Best parameters so far, loglikelihood=%g\n",m_BestLikelihood);
      }
    }
  } else {
    if ( !m_Quiet ) {
      printf("Reject\n");
    }
  }
  if ( !m_Quiet ) {
    std::cerr << std::endl;
  }

  std::vector< double > TElement;
  if ( m_IterationNumber > m_BurnIn ) { // We are just tossing everything in the burn in period.
    if ( m_RescaledTrace ) {
      double* range = new double[2]();
      for ( int k = 0; k < m_CurrentParameters.size(); k++ ) {
        m_Model->GetRange( k, range );
        TElement.push_back( ( m_CurrentParameters[k]-range[0] ) / ( range[1]-range[0] ) );
      }
    } else {
      TElement = m_CurrentParameters;
    }
  }
  if ( m_Debug ) {
    TElement.push_back( m_LikelihoodCurrent );
    TElement.push_back( m_PriorCurrent );
    TElement.push_back( m_ProposalCurrent );
    TElement.push_back( m_ProposalNew );
    TElement.push_back( alpha );
  }
  ThetaOutsList->add( TElement );

  double range[2];
  for ( int k = 0; k < m_Model->GetNumberOfParameters(); k++ ) { //These ParamValus are used for the density plots
    m_Model->GetRange( k, range );
    m_ParameterValues[k] = ( m_CurrentParameters[k] - range[0] ) / ( range[1] - range[0] );
  }

  if ( ( m_IterationNumber > m_BurnIn ) && ( ( m_IterationNumber + 1 ) % ( ThetaOutsList->m_Writeout ) == 0 ) ) {
    std::cout << "Writing out." << std::endl;
    ThetaOutsList->WriteOut( m_Model->GetParameters() );
  }
}

std::vector<double>
MarkovChainMonteCarloSampler
::GetRandomTheta0(int seed)
{ //This creates random theta 0s
  //srand(seed);
  std::cout << "We are using random theta0 values. They are:" << std::endl;
  double *range = new double[2];
  std::vector< double > temp_values( this->m_Model->GetNumberOfParameters(), 0.0 );

  for ( unsigned int i = 0; i < this->m_Model->GetNumberOfParameters(); i++ ) {
    this->m_Model->GetRange( i, range );
    temp_values[i] = double( rand() % int( ( range[1] - range[0] ) * 1000 ) ) / 1000 + range[0];
    std::cerr << temp_values[i] << "  ";
  }
  std::cerr << std::endl;

  return temp_values;
}

std::vector<double>
MarkovChainMonteCarloSampler
::GetTheta0FromFile()
{
  parameterMap parmap;
  std::string theta0_filename = this->m_DirectoryName + "/defaultpars/theta0.param";
  parameter::ReadParsFromFile( parmap, theta0_filename );
  std::vector< std::string > temp_names = parameter::getVS( parmap,"NAMES","" );
  std::vector< double > temp_values = parameter::getV( parmap, "VALUES","" );
  std::vector< double > read_theta;
  unsigned int j, i = 0;

  if ( ! ( ( m_Model->GetParameters()).empty() ) ) {
    double *range = new double[2];
    std::vector< Parameter >::const_iterator itr = (m_Model->GetParameters()).begin();
    std::vector< std::string >::const_iterator titr;
    for ( itr; itr < (m_Model->GetParameters()).end(); itr++ ) {
      j = 0;
      for ( titr = temp_names.begin(); titr < temp_names.end(); titr++ ) {
        if ( ( itr->m_Name ) == *titr ) {
          read_theta.push_back( temp_values[j] );
          break;
        } else {
          j++;
        }
      }
      m_Model->GetRange( i, range );
      if ( read_theta[i] < range[0] ) {
        std::cout << "Parameter below min value.\n\n";
        exit( 1 );
      } else if ( read_theta[i] > range[1] ) {
        std::cout << "Parameter above max value.\n\n";
        exit( 1 );
      }
      i++;
    }
  }
  return read_theta;
}

// Load parameters for taking and evaluating steps
void
MarkovChainMonteCarloSampler
::LoadStepParameters()
{
  m_SepMap = parameter::getB( m_LocalParameterMap, "PROPOSAL_PARAMETER_MAP", false );
  if ( m_SepMap ) {
    std::string parmapfile = m_DirectoryName + "/defaultpars/proposal.param";
    m_StepParameterMap = new parameterMap;
    parameter::ReadParsFromFile( *m_StepParameterMap, parmapfile );
  } else {
    m_StepParameterMap = &( m_LocalParameterMap );
  }

  m_RescaledMethod    = parameter::getB( *m_StepParameterMap, "RESCALED_PROPOSAL", true );
  m_MixingStdDev      = parameter::getV( *m_StepParameterMap, "MIXING_STD_DEV", "0" );
  m_SymmetricProposal = parameter::getB( *m_StepParameterMap, "SYMMETRIC_PROPOSAL", true );
  m_FlatStep          = parameter::getB( *m_StepParameterMap, "FLAT_STEP", false );
  m_Prefactor         = parameter::getD( *m_StepParameterMap, "PREFACTOR", 1.0 );
  m_MinScale          = parameter::getD( *m_StepParameterMap, "MIN", 0.0 );
  m_MaxScale          = parameter::getD( *m_StepParameterMap, "Max", 1.0 );
  m_Scale             = parameter::getD( *m_StepParameterMap, "SCALE", 1.0 );

  const gsl_rng_type * rngtype;
  rngtype = gsl_rng_default;
  gsl_rng_env_setup();
  m_RandNumGen = gsl_rng_alloc( rngtype );
  if ( m_Debug ) {
    gsl_rng_set( m_RandNumGen, 1 );
  } else {
    gsl_rng_set( m_RandNumGen, time( NULL ) );
  }

}

// Take a step in parameter space
std::vector< double >
MarkovChainMonteCarloSampler
::TakeStep( std::vector< double >& current, double& scale )
{
  std::vector< double > proposed = current;
  if ( m_SymmetricProposal ) {
    //We use the scale set in the parameter file
    double range[2];
    for ( int i = 0; i < proposed.size(); i++ ) {
      if ( m_ActiveParameters.find( m_Model->GetParameters()[i].m_Name ) != m_ActiveParameters.end() ) {
        m_Model->GetRange( i, range );
        proposed[i] = (current[i] - range[0])/(range[1]-range[0]); //scale to between 0 and 1
        proposed[i] = proposed[i] + m_Prefactor * gsl_ran_gaussian( m_RandNumGen, m_Scale * m_MixingStdDev[i] );
        proposed[i] = proposed[i] - floor(proposed[i]);
        proposed[i] = ( proposed[i] * ( range[1] - range[0] ) ) + range[0];
      }
    }
  } else if ( m_FlatStep ) {
    double range[2];
    for(int i = 0; i < proposed.size(); i++){
      if(m_ActiveParameters.find(m_Model->GetParameters()[i].m_Name) != m_ActiveParameters.end() ) {
        m_Model->GetRange(i, range);
        proposed[i] = double(rand() % int((range[1] - range[0])*10000))/10000.0+range[0];
      }
    }
  } else {
    // We use whatever scale we just got passed from the rest of the code
    scale = (scale*(m_MaxScale-m_MinScale))+m_MinScale;
    double range[2];
    for( int i=0; i < proposed.size(); i++ ) {
      if ( m_ActiveParameters.find(m_Model->GetParameters()[i].m_Name) !=
           m_ActiveParameters.end() ) {
        m_Model->GetRange( i, range );
        proposed[i] = ( current[i] - range[0])/(range[1]-range[0] ); //scale to between 0 and 1
        proposed[i] = proposed[i] + m_Prefactor * gsl_ran_gaussian( m_RandNumGen, scale*m_MixingStdDev[i] );
        proposed[i] = proposed[i] - floor( proposed[i] );
        proposed[i] = ( proposed[i] * ( range[1]-range[0] ) ) + range[0];
      }
    }
  }
  return proposed;
}

// Calculate the probability of taking the step
double
MarkovChainMonteCarloSampler
::EvaluateProposal( std::vector< double > Theta1,
                    std::vector< double > Theta2,
                    double scale)
{ // At the moment we are using a gaussian proposal distribution, so the scale is the standard deviation
  double probability;
  double exponent = 0, prefactor = 1;

  if ( m_SymmetricProposal || m_FlatStep ) {
    // If it's symmetric this doesn't matter
    probability = 1.0;
  } else {
    scale = (scale*(m_MaxScale-m_MinScale))+m_MinScale;
    for ( int i = 0; i < Theta1.size(); i++ ) {
      exponent += -( Theta1[i] - Theta2[i] ) * ( Theta1[i] - Theta2[i] ) / ( 2*scale*scale*m_MixingStdDev[i]*m_MixingStdDev[i] );
      prefactor = prefactor / (scale * sqrt( 2*M_PI ) );
    }
    probability = prefactor*exp( exponent );
  }
  return probability;
}

} // end namespace madai
