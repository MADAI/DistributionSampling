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
##         https://madai-public.cs.unc.edu/visualization/software-license/
##
##  Unless required by applicable law or agreed to in writing, software
##  distributed under the License is distributed on an "AS IS" BASIS,
##  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
##  See the License for the specific language governing permissions and
##  limitations under the License.
##
##=========================================================================

"""
This is the madai module, which contains classes and functions for use
with the MADAI Distribution Sampling Library
"""

import os, sys, random, signal, glob, subprocess

try:
    import scipy
except ImportError:
    scipy = None

# modify this variable if exe is somewhere else.
GenerateTrainingPointsExecutable = 'madai_generate_training_points'

def GenerateTrainingPoints(parameter_priors_file, directory, N):
    subprocess.call(
        [ GenerateTrainingPointsExecutable, '--format', 'directories',
          parameter_priors_file, directory, str(N)])

class Parameter(object):
    def __init__(self, name, distribution):
        self.Name = name
        self.PriorDistribution = distribution
    def __repr__(self):
        return 'Parameter(%r,%r)' % (self.Name,self.PriorDistribution)
    def __str__(self):
        return '%s\t%s' % (self.Name,self.PriorDistribution)

class Distribution(object):
    @staticmethod
    def Generate(dtype, parameters):
        dtype = dtype.lower()
        if dtype == 'uniform':
            assert len(parameters) == 2
            dist = UniformDistribution()
            dist.Minimum = float(parameters[0])
            dist.Maximum = float(parameters[1])
            return dist
        elif dtype == 'gaussian':
            assert len(parameters) == 2
            dist = GaussianDistribution()
            dist.Mean = float(parameters[0])
            dist.StandardDeviation = float(parameters[1])
            return dist
        raise Exception('Unknown distribution type '+ dtype)

class GaussianDistribution(Distribution):
    def __init__(self,Mean=0.0,StandardDeviation=1.0):
        self.Mean = Mean
        self.StandardDeviation = StandardDeviation
    def __repr__(self):
        return 'GaussianDistribution(%r,%r)' % (
            self.Mean,self.StandardDeviation)
    def __str__(self):
        return 'GAUSSIAN\t%r\t%r' % (self.Mean,self.StandardDeviation)
    def GetInfo(self):
        return ('gaussian', '%r %r' % (self.Mean,self.StandardDeviation))
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
    def GetInfo(self):
        return ('UNIFORM', '%r %r' % (self.Minimum,self.Maximum))
    def GetPercentile(self,percentile):
        return (percentile * (self.Maximum - self.Minimum)) + self.Minimum


def PrintEmulatorFormat(o,X,Y,Parameters,OutputNames,
                        UncertaintyScales=None, ObservedOutputValues=None,
                        comments=[]):
    def print_matrix(M):
        print >>o, '%d %d' % (len(M),len(M[0]))
        for row in M:
            print >>o, '\t'.join(map(repr,row))
    N = len(X) # number of model points
    assert (N > 0) and (len(Y) == N)
    p, t = len(X[0]), len(Y[0])
    assert all(len(x) == p for x in X) and all(len(y) == t for y in Y)
    assert (len(Parameters) == p) and (len(OutputNames) == t)

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
    if ObservedOutputValues is not None:
        print >>o, 'OUTPUT_OBSERVED_VALUES\n%d' % len(ObservedOutputValues)
        for value in ObservedOutputValues:
            print >>o, value
    print >>o, 'END_OF_FILE'

def ReadDirectoryModelFormat(directory):
    def readFile(filename):
        # ignore comments and empty lines
        with open(filename,'r') as f:
            for line in map(lambda s: s.strip(), f):
                if len(line) > 0  and line[0] != '#':
                    yield line
    def makeParameter(s):
        parts = s.strip().split()
        name = parts[1]
        dist = Distribution.Generate(parts[0], parts[2:4])
        return Parameter(name,dist)

    assert os.path.isdir(directory)
    resultsd = os.path.join(directory,'experimental_results')
    analysisd = os.path.join(directory,'statistical_analysis')
    outputd = os.path.join(directory,'model_outputs')
    parameter_priors = os.path.join(analysisd,'parameter_priors.dat')
    observable_names = os.path.join(analysisd,'observable_names.dat')
    experimental_results = os.path.join(resultsd,'results.dat')

    OutputNames = [name for name in readFile(observable_names)]
    Parameters = [makeParameter(x) for x in readFile(parameter_priors)]

    UncertaintyScales = None
    if os.path.isfile(experimental_results):
        ObservedOutputValues = [0.0 for output in OutputNames]
        UncertaintyScales = [0.0 for output in OutputNames]
        for line in readFile(experimental_results):
            name, value, error = line.split()[:3]
            try:
                idx = OutputNames.index(name)
                ObservedOutputValues[idx] = value
                UncertaintyScales[idx] = float(error)
            except ValueError:
                pass # unused output (observable_names.dat)

    X,Y = [],[]
    def getParameterIndex(params,name):
        results = [i for i,p in enumerate(params) if p.Name == name]
        if len(results) == 0:
            raise Exception('no parameter by that name')
        return results[0]

    for rund in sorted(glob.iglob(os.path.join(outputd,"run[0-9]*"))):
        #print >>sys.stderr, rund
        pfile = os.path.join(rund,'parameters.dat')
        rfile = os.path.join(rund,'results.dat')
        assert os.path.isfile(pfile) and os.path.isfile(rfile)

        x = [0.0 for p in Parameters]
        for line in readFile(pfile):
            name, value = line.split()
            index = getParameterIndex(Parameters,name)
            x[index] = float(value)
        X.append(x)

        y = [0.0 for p in OutputNames]
        for line in readFile(rfile):
            values = line.split()
            if len(values) >= 2:
                name, value = values[:2]
            try:
                y[OutputNames.index(name)] = float(value)
            except ValueError:
                pass # unused output (observable_names.dat)
        Y.append(y)
    return {
        'X':X,
        'Y':Y,
        'UncertaintyScales':UncertaintyScales,
        'ObservedOutputValues':ObservedOutputValues,
        'Parameters':Parameters,
        'OutputNames':OutputNames }

def PrintDirectoryModelFormat(
    directory, parameters, outputNames,
    observedValues, observedErrors, N,
    getOutputsAndVariance, comments):
    def mkdir(path):
        try:
            os.mkdir(path)
        except OSError:
            pass # path already exists
    mkdir(directory)
    mkdir(os.path.join(directory, 'statistical_analysis'))
    mkdir(os.path.join(directory, 'model_outputs'))
    mkdir(os.path.join(directory, 'experimental_results'))
    resultsFile = os.path.join(directory, 'experimental_results', 'results.dat')
    with open(resultsFile,'w') as o:
        for value, error, name in zip(
            observedValues, observedErrors, outputNames):
            o.write('%s %r %r\n' % (name, value, error))

    parameter_priors = os.path.join(
        directory, 'statistical_analysis', 'parameter_priors.dat')
    with open(parameter_priors,'w') as o:
        #for comment in model.GetComments():
        #   o.write("# %s\n" % comment)
        for param in parameters:
            dtype,dargs = param.PriorDistribution.GetInfo()
            o.write('%s %s %s\n' % (dtype, param.Name, dargs))

    observable_names = os.path.join(
        directory, 'statistical_analysis', 'observable_names.dat')
    with open(observable_names,'w') as o:
        for comment in comments:
            o.write("# %s\n" % comment)
        for outputName in outputNames:
            o.write('%s\n' % outputName)

    GenerateTrainingPoints(parameter_priors, directory, N)

    def getParameterDictionary(rundir):
        parameter_dictionary = {}
        with open(os.path.join(rundir,'parameters.dat'),'r') as f:
            for line in f:
                line = line.strip()
                if (len(line) > 0) and (line[0] != '#'):
                    name, value = line.split()
                    parameter_dictionary[name] = float(value)
        return parameter_dictionary

    def writeResults(rundir, outputs, outputNames):
        with open(os.path.join(rundir,'results.dat'),'w') as o:
            for value, name in zip(outputs, outputNames):
                o.write('%s %s\n' % (name, value))

    X, Y = [], []
    runDirGlob = os.path.join(directory, 'model_outputs', 'run*')
    for rdir in glob.iglob(runDirGlob):
        pd = getParameterDictionary(rdir)
        for p in parameters:
            if p.Name not in pd:
                raise Exception('missing parameter '+p.Name)
        x = [ pd[p.Name] for p in parameters]
        outputs, errors = getOutputsAndVariance(x)
        assert len(outputs) == len(outputNames)
        writeResults(rdir, outputs, outputNames)
        X.append(x)
        Y.append(outputs)
    return X, Y

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
                        model.GetObservedScalarErrors(),
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

class SettingsFile(object):
    def __init__(self, settings_file):
        if os.path.isdir(settings_file):
            if settings_file[-1] != '/':
                settings_file += '/'
            settings_file += 'settings.dat'
        self.settings_file = settings_file
        with open(settings_file, 'r') as f:
            self.settings = f.read()

    def __repr__(self):
        return self.settings

    def write(self, settings_file = None):
        if settings_file == None:
            settings_file = self.settings_file
        with open(settings_file, 'w') as f:
            f.write(self.settings)

    def set_setting(self, setting, value):
        first_occurance = self.settings.lower().find(setting.lower())
        end_of_line = self.settings.find('\n', first_occurance)
        self.settings = self.settings[:first_occurance + len(setting)] + \
                       ' ' + str(value) + \
                       self.settings[end_of_line:]

    def set_scale(self, scale):
        return self.set_setting('EMULATOR_SCALE', scale)

    def set_nugget(self, scale):
        return self.set_setting('EMULATOR_NUGGET', scale)

class ExperimentalResultsFile(object):
    def __init__(self, results_file):
        self.results_file = results_file
        if os.path.isdir(results_file):
            if results_file[-1] != '/':
                results_file += '/'
            results_file += 'experimental_results.dat'
        self.results = {}
        self.errors = {}
        with open(results_file, 'r') as f:
            for line in f:
                result, value, error = line.split()
                self.results[result] = float(value)
                self.errors[result] = float(error)

class ParametersFile(object):
    def __init__(self, parameters_file):
        self.parameters_file = parameters_file
        if os.path.isdir(parameters_file):
            if parameters_file[-1] != '/':
                parameters_file += '/'
            parameters_file += 'parameter_priors.dat'
        self.parameters = {}
        with open(parameters_file, 'r') as f:
            for line in f:
                type, parameter, x1, x2 = line.split()
                x1, x2 = float(x1), float(x2)
                mean = x1
                sigma = x2
                low, high = x1 - x2, x1 + x2
                if type.lower() == 'uniform':
                    mean = (x1 + x2)/2.0
                    sigma = (x2 - x1)/2.0
                    low, high = x1, x2
                self.parameters[parameter] = {}
                self.parameters[parameter]['type'] = type
                self.parameters[parameter]['mean'] = mean
                self.parameters[parameter]['sigma'] = sigma
                self.parameters[parameter]['low'] = low
                self.parameters[parameter]['high'] = high

    def __repr__(self):
        return str(self.parameters)

    def average(self):
        parameters = {}
        for parameter, values in self.parameters.iteritems():
            parameters[parameter] = values['mean']
        return parameters

    def random(self):
        parameters = {}
        for parameter, values in self.parameters.iteritems():
            if values['type'].lower() == 'uniform':
                parameters[parameter] = random.uniform(values['low'], values['high'])
            elif values['type'].lower() == 'gaussian':
                parameters[parameter] = random.gauss(values['mean'], values['sigma'])
        return parameters

class ModelRun(object):
    def __init__(self, directory):
        if directory[-1] != '/':
            directory += '/'
        self.parameters = {}
        with open(directory + 'parameters.dat', 'r') as f:
            for line in f:
                parameter, value = line.split()
                self.parameters[parameter] = float(value)
        self.outputs = {}
        self.errors = {}
        with open(directory + 'results.dat', 'r') as f:
            for line in f:
                output, value, error = line.split()
                self.outputs[output] = float(value)
                self.errors[output] = float(error)

    def __repr__(self):
        return str({'parameters' : self.parameters, 'outputs' : self.outputs, 'errors' : self.errors})

    def chi_squared(self, **outputs):
        chi_squared = 0.0
        NDF = 0.0
        for output, value in outputs.iteritems():
            chi_squared += ( (value - self.outputs[output]) / self.errors[output] )**2
            NDF += 1.0
        return chi_squared/NDF
