#!/usr/bin/env python
# -*- coding: utf-8 -*-
##=========================================================================
##
##  Copyright 2011-2013 The University of North Carolina at Chapel Hill
##  All rights reserved.
##
##  Licensed under the MADAI Software License. You may obtain a copy of
##  this license at
##
##         https://madai-public.cs.unc.edu/software/license/
##
##  Unless required by applicable law or agreed to in writing, software
##  distributed under the License is distributed on an "AS IS" BASIS,
##  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
##  See the License for the specific language governing permissions and
##  limitations under the License.
##
##=========================================================================

import os, sys, random, signal

try:
    import scipy
except ImportError:
    scipy = None


class Parameter(object):
    def __init__(self, name, distribution):
        self.Name = name
        self.PriorDistribution = distribution
    def __repr__(self):
        return 'Parameter(%r,%r)' % (self.Name,self.PriorDistribution)
    def __str__(self):
        return '%s\t%s' % (self.Name,self.PriorDistribution)

class Distribution(object):
    pass

class GaussianDistribution(Distribution):
    def __init__(self,Mean=0.0,StandardDeviation=1.0):
        self.Mean = Mean
        self.StandardDeviation = StandardDeviation
    def __repr__(self):
        return 'GaussianDistribution(%r,%r)' % (
            self.Mean,self.StandardDeviation)
    def __str__(self):
        return 'GAUSSIAN\t%r\t%r' % (self.Mean,self.StandardDeviation)
    def GetPercentile(self,percentile):
        if scipy is None:
            raise Exception("not implemented yet")
        return scipy.stats.norm.ppf(
            percentile, self.Mean, self.StandardDeviation)

class UniformDistribution(Distribution):
    def __init__(self,Minimum=0.0,Maximum=1.0):
        self.Minimum = Minimum
        self.Maximum = Maximum
    def __repr__(self):
        return 'UniformDistribution(%r,%r)' % (self.Minimum,self.Maximum)
    def __str__(self):
        return 'UNIFORM\t%r\t%r' % (self.Minimum,self.Maximum)
    def GetPercentile(self,percentile):
        return (percentile * (self.Maximum - self.Minimum)) + self.Minimum


def PrintEmulatorFormat(o,X,Y,Parameters,OutputNames,
                        UncertaintyScales=None, comments=[]):
    def print_matrix(M):
        print >>o, '%d %d' % (len(M),len(M[0]))
        for row in M:
            print >>o, '\t'.join(map(repr,row))
    N = len(X) # number of model points
    assert (N > 0) and (len(Y) == N)
    p, t = len(X[0]), len(Y[0])
    assert all(len(x) == p for x in X) and all(len(y) == t for y in Y)
    assert (len(Parameters) == p) and (len(OutputNames) == t)
    assert len(Parameters) == p and len(OutputNames) == t
    for comment in comments:
        print >>o, '# %s' % comment
    print >>o, 'VERSION 1\nPARAMETERS\n%d' % p
    for param in Parameters:
        print >>o, str(param)
    print >>o, 'OUTPUTS\n%d' % (len(Y[0]))
    for output in OutputNames:
        print >>o, output
    print >>o, 'NUMBER_OF_TRAINING_POINTS\t%d' % (N,)
    print >>o, 'PARAMETER_VALUES'
    print_matrix(X)
    print >>o, 'OUTPUT_VALUES'
    print_matrix(Y)
    if UncertaintyScales is not None:
        print >>o, 'OUTPUT_UNCERTAINTY_SCALES\n%d' % len(UncertaintyScales)
        for scale in UncertaintyScales:
            print >>o, scale
    print >>o, 'END_OF_FILE'

def LatinHypercube(parameters, N):
    def sample(param, N):
        samples = [param.PriorDistribution.GetPercentile((0.5 + i)/float(N))
                   for i in xrange(N)]
        random.shuffle(samples)
        return samples
    return zip(*(sample(param,N) for param in parameters))


def GenerateTraining(model, N, output=sys.stdout):
    X = LatinHypercube(model.GetParameters(), N)
    Y = [ model.GetScalarOutputsAndVariance(x)[0] for x in X]
    PrintEmulatorFormat(output,X,Y,
                        model.GetParameters(),
                        model.GetScalarOutputNames(),
                        None,
                        model.GetComments())


def Interactive(Function, Parameters, OutputNames,
                inp=sys.stdin, outp=sys.stdout, comments=[]):
    p = len(Parameters)
    t = len(OutputNames)
    def read_in( readable ):
        while True:
            line = readable.readline()
            if line == '':
                raise StopIteration()
            for token in line.split():
                if token=='STOP':
                    raise StopIteration()
                yield token
    inputs = read_in(inp).next
    for comment in comments:
        outp.write('# %s\n' % comment)
    outp.write('VERSION 1\n')
    outp.write('PARAMETERS\n%d\n' % p)
    for param in Parameters:
        outp.write('%s\n' % str(param))
    outp.write('OUTPUTS\n%d\n' % t)
    for outputName in OutputNames:
        outp.write('%s\n' % outputName)
    outp.write('VARIANCE\n%d\n' % t)
    outp.write('END_OF_HEADER\n')
    outp.flush()
    signal.signal(signal.SIGINT, lambda signal, frame: sys.exit(0))
    while True:
        try:
            params = [inputs() for i in xrange(p)]
            observables, variance = Function(params)
            assert len(observables) == t and len(variance) == t
            for obs in observables:
                outp.write('%r\n' % obs)
            for var in variance:
                outp.write('%r\n' % var)
            outp.flush()
        except (StopIteration,):
            return
        except (KeyboardInterrupt,):
            return
