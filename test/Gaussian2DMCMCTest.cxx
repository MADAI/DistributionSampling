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

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "MarkovChainMonteCarloSampler.h"
#include "Gaussian2DModel.h"

/***
 * Idea for a regression test:
 * Do a large run with the mcmc using a 2D gaussian as the model. Binning the
 * point in parameter space should approximate a gaussian shape.
 ***/
 
bool skip_comments(std::FILE * fp, char comment_character){
  int c = std::getc(fp);
  if((c == EOF) || std::ferror(fp)) {
    std::cerr << "premature endof file:(\n";
    return false;
  }
  while ( c == comment_character ) {
    static int buffersize = 1024;
    char buffer[buffersize];
    std::fgets(buffer, buffersize, fp);
    c = std::getc(fp);
  }
  if(EOF == std::ungetc(c, fp)) {
    std::cerr << "ungetc error :(\n";
    return false;
  }
  return true;
}
 
int main( int argc, char ** argv ) {
  srand( time( NULL ) );
  if( argc != 2 ){
    std::cerr <<
      "Useage:\n\t " << argv[0] << " info_dir_path\n\n"
      "where info_dir_path is the path to the "
      "directory containing all of the configuration "
      "files needed to run the mcmc.\n\n" << std::endl;
    return EXIT_FAILURE;
  }
  std::string info_dir( argv[1] );
  madai::Gaussian2DModel g2d_model;
  madai::MarkovChainMonteCarloSampler run( &g2d_model, info_dir );
  
  std::vector< madai::Parameter > const * parameters = &( g2d_model.GetParameters() );
  for( int i = 0; i < parameters->size(); i++ )
    run.ActivateParameter( (*parameters)[i].m_Name );
  
  madai::Trace trace( info_dir, "default" );
  if( run.m_BurnIn == 0 ) {
    trace.Add( run.m_InitialTheta );
  }
  
  for( int j = 0; j < g2d_model.GetNumberOfParameters(); j++ )
    run.m_ParameterValues.push_back(0);
  
  run.m_AcceptCount = 0;
  for( run.m_IterationNumber = 1; run.m_IterationNumber < trace.m_MaxIterations; run.m_IterationNumber++ ) {
    run.NextSample( &trace );
  }
  if( trace.m_CurrentIteration !=0 ) {
    trace.WriteOut( g2d_model.GetParameters() );
  }
  trace.MakeTrace();
  
  std::cerr << "Trace Created" << std::endl;
  
  // At this point the run has completed. Now I want to bin the points in the trace.
  std::string trace_file_name;
  trace_file_name = trace.m_TraceDirectory.c_str();
  trace_file_name += "/trace.dat";
  FILE* fp = fopen( trace_file_name.c_str(), "r" );
  if( fp == NULL ){
    std::cerr << "Error opening trace.dat [1]" << std::endl;
    return EXIT_FAILURE;
  }
  
  // Find ranges of the trace data
  double range[2][2];
  double x, y;
  int iter;
  range[0][0] = range[1][0] = DBL_MAX;
  range[0][1] = range[1][1] = -DBL_MAX;
  while( !feof(fp) ) {
    skip_comments( fp, '#' );
    fscanf( fp, "%d,%lf,%lf\n", &iter, &x, &y );
    if( x < range[0][0] ) {
      range[0][0] = x;
    } else if ( x > range[0][1] ) {
      range[0][1] = x;
    }
    if( y < range[1][0] ) {
      range[1][0] = y;
    } else if ( y > range[1][1] ) {
      range[1][1] = y;
    }
  }
  range[0][0] -= 0.5;
  range[1][0] -= 0.5;
  range[0][1] += 0.5;
  range[1][1] += 0.5;
  
  std::cerr << "Ranges of MCMC data found" << std::endl;
  
  fclose( fp );
  FILE* tfile = fopen( trace_file_name.c_str(), "r" );
  if( tfile == NULL ) {
    std::cerr << "Error opening trace.dat [2]" << std::endl;
    return EXIT_FAILURE;
  }
  int nbins = 300;
  int** densities = new int*[nbins]();
  for( unsigned int n = 0; n < nbins; n++ ) {
    densities[n] = new int[nbins]();
  }
  // Bin trace into densities
  while( !feof(tfile) ) {
    skip_comments( tfile, '#' );
    fscanf( tfile, "%d,%lf,%lf\n", &iter, &x, &y );
    // scale x and y to [0,nbins)
    x = double( nbins ) * ( x - range[0][0] ) / ( range[0][1] - range[0][0] );
    y = double( nbins ) * ( y - range[1][0] ) / ( range[1][1] - range[1][0] );
    densities[int(x)][int(y)]++;
  }
  // Calculate integrated densities (rho(x) and rho(y))
  int** rho = new int *[2]();
  for( iter = 0; iter < 2; iter++ ) {
    rho[iter] = new int[nbins]();
  }
  for( unsigned int i = 0; i < nbins; i++ ) {
    for( unsigned int j = 0; j < nbins; j++ ) {
      rho[0][i] += densities[i][j];
      rho[1][j] += densities[i][j];
    }
  }
  
  std::cerr << "Trace has been binned into densities" << std::endl;
  
  // Print densities to files
  std::ofstream o1;
  o1.open( "DensityPlot.txt" );
  std::ofstream ox;
  ox.open( "DensityPlotX.txt" );
  std::ofstream oy;
  oy.open( "DensityPlotY.txt" );
  double bin[2], temp, Dev[2];
  int randn=0, alpha;
  int* mb = new int[2]();
  g2d_model.GetDeviations( Dev[0], Dev[1] );
  std::vector< double > tempv;
  bin[0] = ( range[0][1] - range[0][0] ) / double( nbins );
  bin[1] = ( range[1][1] - range[1][0] ) / double( nbins );
  for( unsigned int k = 0; k < nbins; k++ ) {
    ox << range[0][0] + ( double( k ) + 0.5 ) * bin[0] << " " << double( rho[0][k] ) / double( trace.m_MaxIterations ) << std::endl;
    if( rho[0][k] > rho[0][mb[0]] ) { // Find the highest bin count for rhox
      mb[0] = k;
    }
    oy << range[1][0] + ( double( k ) + 0.5 ) * bin[1] << " " << double( rho[1][k] ) / double( trace.m_MaxIterations ) << std::endl;
    if( rho[1][k] > rho[1][mb[1]] ) { // Find the highest bin count for rhoy
      mb[1] = k;
    }
    for( unsigned int l = 0; l < nbins; l++ ) {
      tempv.clear();
      temp = range[0][0] + ( double( k ) + 0.5 ) * bin[0];
      o1 << temp << " ";  // Print x value of center of bin
      tempv.push_back( temp );
      temp = range[1][0] + ( double( l ) + 0.5 ) * bin[1];
      o1 << temp << " ";  // Print y value of center of bin
      tempv.push_back( temp );
      temp = double( densities[k][l] ) / double( trace.m_MaxIterations );
      o1 << temp << std::endl;  // Print density
    }
  }
  o1.close();
  ox.close();
  oy.close();
  for( unsigned int q = 0; q < nbins; q++ ) {
    delete densities[q];
  }
  delete densities;
  
  // Find lower and upper bins at places of the same height on the gaussian (used to estimate the means)
  int** lower_bin = new int*[2]();
  int** upper_bin = new int*[2]();
  int beta;
  for( iter = 0; iter < 2; iter++ ) {
    upper_bin[iter] = new int[10]();
    lower_bin[iter] = new int[10]();
    for( alpha = 0; alpha < 10; alpha++ ) {
      upper_bin[iter][alpha] = nbins;
      lower_bin[iter][alpha] = -1;
    }
  }
  for( beta = 0; beta < 10; beta++ ) {
    for( iter = 0; iter < nbins; iter++ ) {
      for( alpha = 0; alpha < 2; alpha++ ) {
        if( rho[alpha][iter] > int( double ( rho[alpha][mb[alpha]] ) / double( beta+2 ) ) && lower_bin[alpha][beta] == -1 ) {
          lower_bin[alpha][beta] = iter;
        }
        if( rho[alpha][nbins-1-iter] > int( double( rho[alpha][mb[alpha]] ) / double( beta+2 ) ) && upper_bin[alpha][beta] == nbins ) {
          upper_bin[alpha][beta] = nbins - 1 - iter;
        }
      }
      if( lower_bin[0][beta] != -1 && lower_bin[1][beta] != -1 && upper_bin[0][beta] != (nbins-1) && upper_bin[1][beta] != nbins ) {
        break;
      }
    }
  }
  
  std::cerr << "Found upper and lower bins" << std::endl;
  
  // Calculate the Means (Average the upper and lower positions for multiple heights and then average across heights)
  double* Means = new double[2]();
  double** mns = new double*[2]();
  for( iter = 0; iter < 2; iter++ ) {
    mns[iter] = new double[10]();
  }
  double ulpos[2];
  for( iter = 0; iter < 2; iter++ ) {
    for( beta = 0; beta < 10; beta++ ) {
      ulpos[0] = range[iter][0] + ( double( lower_bin[iter][beta] ) + 0.5 ) * bin[iter];
      ulpos[1] = range[iter][0] + ( double( upper_bin[iter][beta] ) + 0.5 ) * bin[iter];
      temp = bin[iter] / double( rho[iter][lower_bin[iter][beta]] - rho[iter][lower_bin[iter][beta] - 1] );
      temp *= ( double( rho[iter][mb[iter]] ) / double( beta + 2 ) - double( rho[iter][lower_bin[iter][beta]-1] ) );
      ulpos[0] = ulpos[0] - bin[iter] + temp;
      temp = bin[iter] / double( rho[iter][upper_bin[iter][beta] + 1] - rho[iter][upper_bin[iter][beta]] );
      temp *= ( double( rho[iter][mb[iter]] ) / double( beta + 2 ) - double( rho[iter][upper_bin[iter][beta]] ) );
      ulpos[1] = ulpos[1] + temp;
      mns[iter][beta] = ( ulpos[0] + ulpos[1] ) / 2.0;
      Means[iter] += ( ulpos[0] + ulpos[1] ) / 2.0;
    }
    Means[iter] /= 10.0;
  }
  
  std::cerr << "Means Calculated" << std::endl;
  
  double* error = new double[2]();
  for( iter = 0; iter < 2; iter++ ) {
    for( beta = 0; beta < 10; beta++ ) {
      error[iter] += ( Means[iter] - mns[iter][beta] ) * ( Means[iter] - mns[iter][beta] );
    }
    error[iter] /= 10;
    error[iter] = sqrt( error[iter] );
  }
  
  std::cerr << "Means Errors Calculated" << std::endl;
  
  // Mean bins
  mb[0] = int( double( nbins ) * ( Means[0] - range[0][0] ) / ( range[0][1] - range[0][0] ) );
  mb[1] = int( double( nbins ) * ( Means[1] - range[1][0] ) / ( range[1][1] - range[1][0] ) );
  
  std::cerr << "Got Mean bins" << std::endl;
  
  // Initial values of the deviations
  double** sigma = new double*[2]();
  for( iter = 0; iter < 2; iter++ ) {
    double r = double( rand() % ( int( Dev[iter] ) * 150000 ) ) / 50000.0;
    sigma[iter] = new double[3]();
    sigma[iter][0] = r;
    sigma[iter][1] = 0;
    sigma[iter][2] = 3 * Dev[iter];
  }
  
  std::cerr << "Initialized sigmas" << std::endl;
  
  // Begin optimizing the gaussian as a function of sigma
  double prec = -16.0;
  double* S = new double[2]();
  bool* done = new bool[2]();
  for( iter = 0; iter < 2; iter++ ) {
    done[iter] = false;
    S[iter] = 1;
  }
  int iteration=0;
  std::cerr << "X Normalization = " << double( rho[0][mb[0]] ) / trace.m_MaxIterations << std::endl;
  std::cerr << "Y Normalization = " << double( rho[1][mb[1]] ) / trace.m_MaxIterations << std::endl;
  std::cerr << "Begin optimizing the summed difference between a gaussian fit and the data" << std::endl;
  while( abs(S[0] ) > pow( 10, prec ) || abs( S[1] ) > pow( 10, prec ) ) {
    iteration++;
    std::cerr << "Iteration " << iteration << " of optimization of fit" << std::endl;
    std::cerr << "sigmax = " << sigma[0][0] << " sigmay = " << sigma[1][0] << std::endl;
    S[0] = S[1] = 0;
    for( unsigned int iters = 0; iters < nbins; iters++ ) {
      for( iter = 0; iter < 2; iter++ ) {
        double se[4];
        if( !done[iter] ) {
          se[0] = double( rho[iter][mb[iter]] ) / double( trace.m_MaxIterations ); //Normalization
          se[1] = range[iter][0] + ( double ( iters ) + 0.5 ) * bin[iter] - Means[iter];
          se[1] *= se[1];
          se[1] /= ( 2 * sigma[iter][0] * sigma[iter][0] );
          se[1] = exp( -se[1] );
          se[2] = double( rho[iter][iters] ) / double( trace.m_MaxIterations );
          se[3] = se[0] * se[1] - se[2];
          S[iter] += se[3];
        }
      }
    }
    for( iter = 0; iter < 2; iter++ ) {
      if( S[iter] > pow( 10, prec ) ) {
        if( sigma[iter][0] < sigma[iter][2] ) {
          sigma[iter][2] = sigma[iter][0];
        }
        sigma[iter][0] = ( sigma[iter][0] + sigma[iter][1] ) / 2.0;
      } else if( S[iter] < ( -pow( 10, prec ) ) ) {
        if( sigma[iter][0] > sigma[iter][1] ) {
          sigma[iter][1] = sigma[iter][0];
        }
        sigma[iter][0] = ( sigma[iter][0] + sigma[iter][2] ) / 2.0;
      }
      std::cerr << "Sum of differences: " << S[iter] << std::endl;
    }
    std::cerr << std::endl;
  }
  
  // Print out fit and model values for inspection
  std::cerr << "Parameters for the gaussian fit of the densities:" << std::endl;
  double ModMeanX, ModMeanY;
  g2d_model.GetMeans( ModMeanX, ModMeanY );
  std::cerr << "XMean: Fit = " << Means[0] << "  Error = " << error[0] << "  Model = " << ModMeanX << std::endl;
  std::cerr << "XSigma: Fit = " << sigma[0][0] << "  Model = " << Dev[0] << std::endl;
  std::cerr << "YMean: Fit = " << Means[1] << "  Error = " << error[1] << "  Model = " << ModMeanY << std::endl;
  std::cerr << "YSigma: Fit = " << sigma[1][0] << "  Model = " << Dev[1] << std::endl;
  
  // Calculate various values which might shine light on how well a fit the gaussian is
  double* peak_abs_dev = new double[2]();
  double* peak_perc_dev = new double[2]();
  double* SPD = new double[2]();
  int used_bins=0;
  S[0] = S[1] = 0;
  for( unsigned int alpha = 0; alpha < nbins; alpha++ ) {
    for( iter = 0; iter < 2; iter++ ) {
      double se[4];
      if( !done[iter] ) {
        if( rho[iter][alpha] > int( double( rho[iter][mb[iter]] ) / 100.0 ) ) {
          se[0] = double( rho[iter][mb[iter]] ) / double( trace.m_MaxIterations ); //Normalization
          se[1] = range[iter][0] + ( double( alpha ) + 0.5 ) * bin[iter] - Means[iter];
          se[1] *= se[1];
          se[1] /= ( 2 * sigma[iter][0] * sigma[iter][0] );
          se[1] = exp( -se[1] );
          se[2] = double( rho[iter][alpha] ) / double( trace.m_MaxIterations );
          se[3] = abs( se[0] * se[1] - se[2] );
          if( se[3] > peak_abs_dev[iter] ) {
            peak_abs_dev[iter] = se[3];
          }
          if( se[3] / ( se[0] * se[1] ) > peak_perc_dev[iter] ) {
            peak_perc_dev[iter] = se[3] / ( se[0] * se[1] );
          }
          SPD[iter] += se[3] / ( se[0] * se[1] );
          used_bins++;
          S[iter] += se[3];
        }
      }
    }
  }
  std::cerr << "\nThe peak absolute deviation from rhox is:\n" << peak_abs_dev[0] << std::endl;
  std::cerr << "The peak percentage deviation from rhox is:\n" << peak_perc_dev[0] << std::endl;
  std::cerr << "\nThe peak absolute deviation from rhoy is:\n" << peak_abs_dev[1] << std::endl;
  std::cerr << "The peak percentage deviation from rhoy is:\n" << peak_perc_dev[1] << "\n\n";
  std::cerr << "The average absolute deviations:\nrhox = " << S[0] / double( used_bins ) << "\nrhoy = " << S[1] / double( used_bins ) << std::endl;
  std::cerr << "The average percentage deviations:\nrhox = " << SPD[0] / double( used_bins ) << "\nrhoy = " << SPD[1] / double( used_bins ) << std::endl;
  
  for( iter = 0; iter < 2; iter++ ) {
    delete rho[iter];
  }
  delete rho;
  
  // Open pipe to create plots of rho(x) and rho(y)
  FILE* XPlot = popen( "gnuplot -persist", "w" );
  bool PlotDensities = true;
  if( !XPlot ) {
    std::cerr << "GNUPlot not found :(" << std::endl;
    PlotDensities = false;
    //exit(1);
  }
  FILE* YPlot = popen( "gnuplot -persist", "w" );
  if( !YPlot ) {
    std::cerr << "GNUPlot not found :(" << std::endl;
    PlotDensities = false;
    //exit(1);
  }
  if( PlotDensities ) { // Plot rhox and rhoy
    std::string command1;
    std::string command2;
    fprintf( XPlot, "%s\n", "set term x11" );
    fprintf( XPlot, "%s\n", "set xlabel \"X\"" );
    fprintf( XPlot, "%s\n", "set ylabel \"Density\"" );
    fprintf( XPlot, "%s\n", "set title \"Density Over X\"" );
    fprintf( XPlot, "%s\n", "set key out vert right top" );
    command1 = "plot \"DensityPlotX.txt\" t \"rho(x)\" w lines";
    fprintf( XPlot, "%s\n", command1.c_str() );
    fprintf( YPlot, "%s\n", "set term x11" );
    fprintf( YPlot, "%s\n", "set ylabel \"Density\"" );
    fprintf( YPlot, "%s\n", "set xlabel \"Y\"" );
    fprintf( YPlot, "%s\n", "set title \"Density Over Y\"" );
    fprintf( YPlot, "%s\n", "set key out vert right top" );
    command2 = "plot \"DensityPlotY.txt\" t \"rho(y)\" w lines";
    fprintf( YPlot, "%s\n", command2.c_str() );
  } else {
    std::cerr << "This test is set up to plot the densities in GNUPlot" << std::endl;
    std::cerr << "Densities have been written to DensityPlot.txt, DensityPlotX.txt, and DensityPlotY.txt" << std::endl;
    std::cerr << "Plot these values for inspection of the distribution" << std::endl;
  }
  
  return EXIT_SUCCESS;
}
