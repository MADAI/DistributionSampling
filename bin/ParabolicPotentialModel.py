#!/usr/bin/python -tt

# -*- coding: utf-8 -*-
"""

Parabolic Potential Model
Copyright 2012-2013, Michigan State University.

The software was written in 2013 by Jeffrey Wyka
while working for the MADAI project <http://madai.us/>

See <https://madai-public.cs.unc.edu/software/license/> for license
information.

This is inteded as an example of:

- A parameterized model of a physical system with a parabolic potential

"""

import math
import sys
import madai
import inspect

class ParabolicPotentialModel(object):
    @staticmethod
    def GetComments():
        return [inspect.getfile(inspect.currentframe())]
    @staticmethod
    def GetParameters():
        return [
            madai.Parameter('X0', madai.UniformDistribution(-2.0,2.0)),
            madai.Parameter('K', madai.UniformDistribution(0.5,4.0)),
            madai.Parameter('TEMP', madai.UniformDistribution(0.5,10.0))]
    @staticmethod
    def GetScalarOutputNames():
        return [
            'MEAN_X',
            'MEAN_X_SQUARED',
            'MEAN_ENERGY']
    @staticmethod
    def GetScalarOutputsAndVariance(x):
        """
        Parabolic Potential Model
        Copyright 2012-2013, Michigan State University.

        The software was written in 2013 by Jeffrey Wyka
        while working for the MADAI project <http://madai.us/>

        See <https://madai-public.cs.unc.edu/software/license/> for license
        information.

        This is inteded as an example of:
        A parameterized model of a physical system with a parabolic potential
        """

        for i in xrange(1,3):
            if float( x[i] ) <= 0:
                raise Exception("Parameter %d cannot be non-positive" % (i+1))

        x0 = float(x[0])
        k = float(x[1])
        T = float(x[2])

        def GetGaussianIntegral( power, scale  ):
            n = int(power)
            alpha = float(scale)
            Integral = float(1.0)
            if n % 2 == 1:
                Integral = 0.0
            elif n % 2 == 0:
                for i in xrange(n/2, n):
                    Integral *= float(i + 1)
                Integral *= (2.0 * math.sqrt(3.14159265))
                Integral /= ( 2.0 * math.sqrt(alpha) )**( n+1 )
            return Integral

        # Calculate the normalization
        # Integrate( e^( -k/T*(x-x0)^2 ) )
        normalization = float(GetGaussianIntegral(0, k/T))

        # Calculate Mean X
        # Integrate( x e^( -k/T*(x - x0)^2 ) ) / normalization
        MeanX = (float(GetGaussianIntegral(1, k/T))
                 + x0*float(GetGaussianIntegral(0, k/T)))
        MeanX /= normalization

        # Calculate Mean X^2
        # Integrate( x^2 e^( -k/T*(x - x0)^2 ) ) / normalization
        MeanX2 = float(GetGaussianIntegral(2, k/T))
        MeanX2 += 2.0*x0*float(GetGaussianIntegral(1, k/T))
        MeanX2 += x0*x0*float(GetGaussianIntegral(0, k/T))
        MeanX2 /=normalization

        # Calculate Mean Energy
        # Integrate( k (x - x0)^2 * e^( -k/T*(x - x0)^2 ) ) /
        #                      normalization + T/2
        MeanE = k*float(GetGaussianIntegral(2, k/T))/normalization + T/2.0

        # Calculate Mean X^4 for getting the model error
        # Integrate( x^4 e^( -k/T*(x-x0)^2 ) ) / normalization
        MeanX4 = float(GetGaussianIntegral(4, k/T))
        MeanX4 += 4.0*x0*float(GetGaussianIntegral(3, k/T))
        MeanX4 += 6.0*x0*x0*float(GetGaussianIntegral(2, k/T))
        MeanX4 += 4.0*(x0**3)*float(GetGaussianIntegral(1, k/T))
        MeanX4 += (x0**4)*float(GetGaussianIntegral(0, k/T))
        MeanX4 /= normalization

        # Calculate Mean Energy^2 for getting the model error
        # Integrate( ( k^2*(x - x0)^4 + k*T*(x - x0)^2 )
        #                       * e^( -k/T*(x-x0)^2 ) ) + 0.25*T^2
        MeanE2 = k*k*float(GetGaussianIntegral(4, k/T))/normalization
        MeanE2 += k*T*float(GetGaussianIntegral(2, k/T))/normalization
        MeanE2 += T*T/4.0

        # Calculate Errors
        ErrorX = (MeanX2 - MeanX**2)**(0.5)
        ErrorX2 = (MeanX4 - MeanX2**2)**(0.5)
        ErrorE = (MeanE2 - MeanE**2)**(0.5)

        return ( [ MeanX, MeanX2, MeanE], [ErrorX, ErrorX2, ErrorE ])

if __name__=='__main__':
    madai.Interactive(ParabolicPotentialModel.GetScalarOutputsAndVariance,
                      ParabolicPotentialModel.GetParameters(),
                      ParabolicPotentialModel.GetScalarOutputNames(),
                      sys.stdin,
                      sys.stdout,
                      ParabolicPotentialModel.GetComments())

      Integral *= float(i + 1)
    Integral *= (2.0 * math.sqrt(3.14159265))
    Integral /= ( 2.0 * math.sqrt(alpha) )**( n+1 )
  return Integral

