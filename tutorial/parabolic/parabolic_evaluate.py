#!/usr/bin/env python
import os, glob, sys

from ParabolicPotentialModel import ParabolicPotentialModel

def process_model_output_directory(model, rundir):
	parameter_names = [p[0] for p in model.Parameters]
	output_names = model.Outputs
	params = [0.0 for p in parameter_names]
	parameters_file = os.path.join(rundir, 'parameters.dat')
	if not os.path.isfile(parameters_file):
		raise IOError('"%s" does not exist!' % parameters_file)
	with open(parameters_file,'r') as f:
		for line in f:
			line = line.strip()
			if len(line) > 0 and line[0] != '#':
				name, value = line.split()[:2]
				index = parameter_names.index(name)
				params[index] = value
	outputs, errors = model.Run(params)
	with open(os.path.join(rundir, 'results.dat'), 'w') as o:
		for name, value, error in zip(output_names, outputs, errors):
			print >>o, name, value, error

if len(sys.argv) == 1:
	print 'useage: %s DIRECTORY [MORE DIRECTORIES...]' % sys.argv[0]
	exit(1)
model = ParabolicPotentialModel()
for arg in sys.argv[1:]:
	for path in glob.glob(arg):
		if os.path.isdir(path):
			try:
				process_model_output_directory(model, path)
				print '"%s" processed successfully' % path
			except:
				print 'ERROR PROCESSING "%s"!' % path
				raise
		else:
			print '"%s" is not a directory' % path
