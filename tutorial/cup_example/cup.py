#!/usr/bin/env python
# -*- coding: utf-8 -*-
##=========================================================================
##
##  Copyright 2011-2013 The University of North Carolina at Chapel Hill
##  and Michigan State University.  All rights reserved.
##
##  Licensed under the MADAI Software License. You may obtain a copy of
##  this license at
##
##    https://madai-public.cs.unc.edu/visualization/software-license/
##
##  Unless required by applicable law or agreed to in writing, software
##  distributed under the License is distributed on an "AS IS" BASIS,
##  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
##  See the License for the specific language governing permissions and
##  limitations under the License.
##
##=========================================================================
import os, glob, math, sys, signal

class CModel(object):
	"""
	has an intersting likelihood function
	"""
	Parameters = [
		('x1',   'UNIFORM',   (-1.2,    1.2)),
		('x2',   'UNIFORM',   (-1.2,    1.2)),
		('x3',   'UNIFORM',   (-0.5,    1.9)),
		('x4',   'UNIFORM',   (-0.8,    0.8))]
	Outputs = ['y1', 'y2','y3', 'y4', 'y5']
	@staticmethod
	def Run(x):
		x = map(float,x)
		sq = lambda v: v**2
		y = [0.0 for i in xrange(5)]
		yerror = [0.0 for i in xrange(5)]
		y[0] = x[0]
		y[1] = x[1]
		y[2] = x[2]
		y[3] = 7 * x[3];
		y[4] = 10 * (sq(x[0]) + sq(x[1]) - x[2]);
		return ( y, yerror)

def Interact(model):
	def tok(readable):
		while True:
			try:
				line = readable.readline()
			except:
				return
			if line == '':
				return
			for token in line.split():
				if token == "STOP":
					return
				yield token
	o = sys.stdout
	it = tok(sys.stdin)
	parameters = model.Parameters
	outputs = model.Outputs
	o.write('VERSION 1\nPARAMETERS %d\n' % len(parameters))
	for pname, ptype, arguments in parameters:
		o.write('%s\t%s\t%s\n' %(pname, ptype, '\t'.join(map(repr, arguments))))
	o.write('OUTPUTS %d\n' % len(outputs))
	for output in outputs:
		o.write('%s\n' % output)
	o.write('VARIANCE %d\nEND_OF_HEADER\n' % len(outputs))
	o.flush()
	nparams = len(parameters)
	while True:
		try:
			x = [it.next() for i in xrange(nparams)]
		except (StopIteration,):
			return
		else:
			v,e = model.Run(x)
			assert len(v) == len(e) == len(outputs)
			o.write(' '.join(map(repr, v)))
			o.write('\n')
			o.write(' '.join(map(repr, e)))
			o.write('\n')
			o.flush()

if __name__ == '__main__':
	signal.signal(signal.SIGINT, lambda *a : sys.exit(0))
	Interact(CModel())
