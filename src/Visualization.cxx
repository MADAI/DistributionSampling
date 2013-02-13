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

#include "Visualization.h"

#include "MCMCRun.h"

#include <deque>


namespace madai {


Visualization
::Visualization( MCMCRun *mcmc_in )
{
  m_MCMC = mcmc_in;

  if ( !m_MCMC ) {
    std::cout << "MCMC not loaded!" << std::endl;
    exit( 1 );
  }

  if ( m_MCMC->m_VizTrace ) {
    // Setting up the pipe for the trace plot
    m_GNUPlotPipe = popen("gnuplot -persist", "w");
    if ( !m_GNUPlotPipe ) {
      std::cout << "Gnuplot not found!" << std::endl;
      exit( 1 );
    }
    m_GNUPlotTerm = parameter::getS( m_MCMC->m_LocalParameterMap, "GNUPLOT_TERMINAL", "x11" );
    m_GNUPlotStyle = parameter::getS( m_MCMC->m_LocalParameterMap, "GNUPLOT_TRACESTYLE", "linespoints" );

    fprintf( m_GNUPlotPipe, "%s\n", ("set term " + m_GNUPlotTerm).c_str() );
    fprintf( m_GNUPlotPipe, "%s\n", "set xlabel 'iteration'" );
    fprintf( m_GNUPlotPipe, "%s\n", "set ylabel 'parameter value'" );
    fprintf( m_GNUPlotPipe, "%s\n", "set key out vert right top" );
    fflush( m_GNUPlotPipe);


    // JFN 7/2/12: Creating the density plots live while the MCMC runs slows things down a lot. So,
    // I've adopted a system where they are just printed to file, and you can plot them with an external script.

    // Setting up the pipe for the density plots
    /*gnuplotmultipipe = popen("gnuplot -persist", "w");
      if(!gnuplotmultipipe){
      cout << "Gnuplot not found!" << endl;
      exit(1);
      }

      stringstream ss;
      ss << "set term " << gnuplotterm << "\n set multiplot\n";

      fprintf(gnuplotmultipipe, "%s\n", ss.str().c_str());
      fflush(gnuplotmultipipe);
    */
  }

  // The header for the trace plot
  unsigned int N = m_MCMC->GetModel()->GetNumberOfParameters();
  m_Header = "plot ";
  for ( int i = 0; i < N; i++ ) {
    m_Header = m_Header + "'-' w " + m_GNUPlotStyle + " t '" + m_MCMC->GetModel()->GetParameters()[i].m_Name+ "'";
    if ( i != N-1 ) {
      m_Header = m_Header + ", ";
    }
  }
  m_Header = m_Header + "\n";

  //Check if DensityPlots file exists
  std::string tempfile;
  struct stat st;
  tempfile = m_MCMC->GetModel()->m_DirectoryName + "/DensityPlots";
  if ( stat( tempfile.c_str(), &st ) != 0 ) {
    std::string command = "mkdir -p " + tempfile;
    std::system( command.c_str() );
  }

  // The plot commands for the density plots
  for ( int i = 0; i < N; i++ ) {
    //cout << mcmc->ThetaList->ParamNames[i] << endl;
    std::stringstream command;
    command << "clear\nset size square " << 1./float(N-2) << "," << 1./float(N-2) << "\n";
    for ( int j = i+1; j < N; j++ ) {
      //cout << "     " << mcmc->ThetaList->ParamNames[j] << endl
      //Now we mke our 2d plots
      std::string filename = m_MCMC->GetModel()->m_DirectoryName + "/DensityPlots/" + m_MCMC->GetModel()->GetParameters()[i].m_Name
        + "_" + m_MCMC->GetModel()->GetParameters()[j].m_Name;
      m_DensityPlotFileNames.push_back( filename );
      command << "set bmargin 0.001\n set tmargin 0.001\n set lmargin 0.001\n set rmargin 0.001\nunset key\nunset xtics\nunset ytics\n";
      command << "\nset origin " << (float(i)*(1./float(N))) << "," << (float(j-1)*(1./float(N))) << "\n";
      command << "set xlabel \'" << m_MCMC->GetModel()->GetParameters()[i].m_Name
              << "\'\nset ylabel \'" << m_MCMC->GetModel()->GetParameters()[i].m_Name
              << "\'\nset view map\nsplot \'" << m_DensityPlotFileNames.back()
              << ".txt\' matrix with image\n";
      m_DensityPlotCommands.push_back( command.str() );
      //cout << command.str();
      command.str("");
      if ( m_MCMC->m_AppendTrace ) {
        std::fstream densityfile;
        std::string line,val;
        std::stringstream ss2;
        filename = filename + ".txt";
        densityfile.open( filename.c_str() );
        if ( densityfile ) {
          for ( int l = 0; l < 100; l++ ) {
            std::getline( densityfile, line, '\n' );
            ss2 << line;
            for ( int k = 0; k < 100; k++ ) {
              std::getline( ss2, val, ' ' );
              m_Densities[i][j][l][k] = atoi( val.c_str() );
            }
          }
        }
      } else {
        for ( int l = 0; l < 100; l++ ) {
          for ( int k = 0; k < 100; k++ ) {
            m_Densities[i][j][l][k] = 0;
          }
        }
      }
    }
  }

  //cout << "The gnuplot header is: \n" << header << endl;

  m_ParamValues = new std::string[N];
  m_DequeParameterValues = new deque<std::string>[N];
  m_MovingWindow = true; //This should be made into an option which is set in mcmc.param
  m_DensityPlot = true; //Again, this should probably be set in a parameter file
  m_HighestItnReadIn = 0;
  m_Bins = 100;
}


Visualization
::~Visualization()
{
  if ( m_MCMC->m_VizTrace ) {
    fprintf(m_GNUPlotPipe, "exit\n");
    pclose(m_GNUPlotPipe);
    //fprintf(gnuplotmultipipe, "exit\n");
    //pclose(gnuplotmultipipe);
  }
}

// KMN not sure if I'm going to use this, it should allow for something like:
//Visualization plotter(this);
//plotter("plot cos(x)");
void
Visualization
::operator() ( const std::string& command )
{
  if ( m_MCMC->m_VizTrace ) {
    fprintf( m_GNUPlotPipe, "%s\n", command.c_str() );
    fflush( m_GNUPlotPipe );
  }
}


void
Visualization
::UpdateTraceFig( Trace* ThetaOutsList )
{
  std::stringstream ss;
  std::string plotcommand = m_Header;
  std::vector<std::string> m_ParamNames;
  const std::vector< Parameter > parameters = m_MCMC->GetModel()->GetParameters();
  for ( int i = 0; i < parameters.size(); i++ ) {
    m_ParamNames.push_back( parameters[i].m_Name );
  }

  if ( m_MCMC->m_VizTrace ) {
    if ( m_MovingWindow ) {
      int DequeSize = 500;
      for ( int i = 0; i < ThetaOutsList->m_Writeout; i++ ) {
        if ( (*ThetaOutsList)[i].m_Used && !((*ThetaOutsList)[i].m_InTrace) ) {
          for ( int j = 0; j< (*ThetaOutsList)[i].m_ParameterValues.size(); j++ ) {
            if ( m_MCMC->m_RescaledTrace ) {
              double * range = new double[2]();
              m_MCMC->GetModel()->GetRange( j, range );
              ss << ThetaOutsList->m_Writeout*ThetaOutsList->m_WriteOutCounter + i + 1 << " "
                 << ((*ThetaOutsList)[i].m_ParameterValues[j]-range[0])/(range[1]-range[0])<< "\n";
            } else {
              ss << ThetaOutsList->m_Writeout*ThetaOutsList->m_WriteOutCounter + i + 1 << " " << (*ThetaOutsList)[i].m_ParameterValues[j] << "\n";
            }
            if ( m_DequeParameterValues[j].size() > DequeSize ) {
              m_DequeParameterValues[j].pop_front();
              m_DequeParameterValues[j].push_back( ss.str() );
            } else {
              m_DequeParameterValues[j].push_back( ss.str() );
            }
            ss.str( string() ); //clears the stringstream.
          }
          (*ThetaOutsList)[i].VizTrace();
        }
      }
      for ( int i = 0; i < m_ParamNames.size(); i++ ) {
        for ( int j = 0; j < m_DequeParameterValues[i].size(); j++ ) {
          plotcommand = plotcommand + (m_DequeParameterValues[i])[j];
        }
        plotcommand = plotcommand + "e\n";
        //std::cerr << plotcommand << std::endl;
      }
    } else {
      if ( !m_MCMC ) {
        std::cout << "MCMC pointer not found." << std::endl;
      }
      for ( int i = 0; i < ThetaOutsList->m_Writeout; i++ ) {
        if ( (*ThetaOutsList)[i].m_Used && !((*ThetaOutsList)[i].m_InTrace) ) {
          // count++;
          for ( int j =0; j < (*ThetaOutsList)[i].m_ParameterValues.size(); j++ ) {
            if ( m_ParamValues[j].empty() ) {
              m_ParamValues[j] = "";
            }
            if ( m_MCMC->m_RescaledTrace ) {
              double * range = new double[2]();
              m_MCMC->GetModel()->GetRange( j, range );
              ss << ThetaOutsList->m_Writeout*ThetaOutsList->m_WriteOutCounter + i + 1 << " "
                 << ((*ThetaOutsList)[i].m_ParameterValues[j]-range[0])/(range[1]-range[0])<< "\n";
            } else {
              ss << ThetaOutsList->m_Writeout*ThetaOutsList->m_WriteOutCounter + i + 1 << " " << (*ThetaOutsList)[i].m_ParameterValues[j] << "\n";
            }
            m_ParamValues[j] = ss.str();
            ss.str( string() ); //clears the stringstream.
          }
          (*ThetaOutsList)[i].VizTrace();
        }
      }
      for ( int i = 0; i < m_ParamNames.size(); i++ ) {
        plotcommand = plotcommand + m_ParamValues[i]+ "e\n";
        //cout << paramvalues[i] << endl;
      }
    }
    //std::cerr << "The plot command is: \n" << plotcommand << std::endl;
    fprintf( m_GNUPlotPipe, "%s", plotcommand.c_str() );
    fflush( m_GNUPlotPipe );
  }

  if ( m_DensityPlot ) {
    int index = 0;
    for ( int i = 0; i < m_ParamNames.size(); i++ ) {
      for ( int j = i+1; j < m_ParamNames.size(); j++ ) {
        m_Densities[i][j][int(m_MCMC->m_ParameterValues[i]*100)][int(m_MCMC->m_ParameterValues[j]*100)]++;
        ofstream outputfile;
        string filename = m_DensityPlotFileNames[index] + ".txt";
        outputfile.open( filename.c_str() );
        for ( int k = 0; k < m_Bins; k++ ) {
          for ( int l = 0; l < m_Bins; l++ ) {
            outputfile << m_Densities[i][j][k][l] << " ";
          }
          outputfile << std::endl;
        }
        outputfile.close();
        //cout << DensityPlotCommands[index].c_str() << endl;
        //fprintf(gnuplotmultipipe, "%s", DensityPlotCommands[index].c_str());
        //fflush(gnuplotmultipipe);
        index++;
      }
    }
  }
}


void
Visualization
::FinalTrace( Trace* ThetaOutsList )
{
  if ( m_MCMC->m_VizTrace ) {
    std::vector< std::string > m_ParamNames;
    const std::vector< Parameter > parameters = m_MCMC->GetModel()->GetParameters();
    for( int i = 0; i < parameters.size(); i++ ) {
      m_ParamNames.push_back( parameters[i].m_Name );
    }
    stringstream ss;

    fprintf( m_GNUPlotPipe,"%s\n","set datafile separator ','" );
    fflush( m_GNUPlotPipe );
    ss << "plot '"<< ThetaOutsList->m_TraceDirectory << "/trace.dat' using 1:2 w " << m_GNUPlotStyle << " t '" << m_ParamNames[0] << "', ";
    for ( int i = 1; i < m_ParamNames.size(); i++ ) {
      ss<< "'' u 1:"<< i+2 <<" w "<< m_GNUPlotStyle << " t '"<< m_ParamNames[i];
      if ( i != m_ParamNames.size()-1 ) {
        ss << "', ";
      }
    }
    string gnuplotcmd = ss.str() + "\n";

    fprintf( m_GNUPlotPipe, "%s", gnuplotcmd.c_str() );
    fflush( m_GNUPlotPipe );

    ss.str( string() ); //clears the stringstream

    ss << "set term postscipt\n" << "set ouput " << ThetaOutsList->m_TraceDirectory << "/trace.ps\n"
       << "replot\n"
       << "set term x11\n"; // This last may need to be changed to "set term win" on windows machines
    fprintf( m_GNUPlotPipe, "%s", gnuplotcmd.c_str() );
    fflush( m_GNUPlotPipe );
  }
}

} // end namespace madai
